// pomodoro.cpp
// terminal pomodoro timer — hourglass + big-digit clock side by side
//
// keys:
//   space  - pause / resume
//   s      - skip to next phase
//   r      - reset current timer
//   q      - quit

#include <algorithm>
#include <chrono>
#include <cmath>
#include <csignal>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <conio.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

// ─── stevans request-- new user config for colours (color)
// ───────────────────────────────

namespace config {
//
struct Color {
  int r, g, b;
};
struct Theme {
  Color work, short_b, long_b;
};

// theme colours (rgb values (can change))
constexpr Theme THEME_AMBER = {{220, 140, 40}, {70, 210, 130}, {80, 160, 240}};
constexpr Theme THEME_TRON = {{0, 255, 255}, {0, 150, 255}, {0, 50, 255}};

// ---- EDIT THIS FOR YOUR CUSTOM THEME ----
constexpr Theme STEVAN_MODE = {{0, 255, 255}, {0, 150, 255}, {0, 50, 255}};

// mutable active color theme ( default is amber )
static Theme active = THEME_AMBER;

constexpr Color GLASS = {170, 160, 145};
constexpr Color GRAIN = {255, 230, 130};

} // namespace config

// ─── ansi helpers ────────────────────────────────────────────────────────────

namespace ansi {

// position cursor at row r, col c (1-indexed)
inline std::string mv(int r, int c) {
  char b[24];
  snprintf(b, sizeof b, "\033[%d;%dH", r, c);
  return b;
}

// 24-bit foreground colour (raw integer interface)
inline std::string rgb(int r, int g, int b) {
  char buf[32];
  snprintf(buf, sizeof buf, "\033[38;2;%d;%d;%dm", r, g, b);
  return buf;
}

// 24-bit foreground colour (struct interface)
inline std::string rgb(config::Color c) { return rgb(c.r, c.g, c.b); }

constexpr const char *RST = "\033[0m";
constexpr const char *BOLD = "\033[1m";
constexpr const char *DIM = "\033[2m";
constexpr const char *CLR = "\033[2J\033[H";
constexpr const char *HIDE = "\033[?25l";      // hide cursor
constexpr const char *SHOW = "\033[?25h";      // show cursor
constexpr const char *EL = "\033[K";           // erase rest of line
constexpr const char *ALT_ON = "\033[?1049h";  // use alternate screen buffer
constexpr const char *ALT_OFF = "\033[?1049l"; // restore main screen buffer

// repeat a utf-8 string n times (can't use std::string(n, char) for multi-byte
// glyphs)
inline std::string rep(const char *s, int n) {
  std::string out;
  out.reserve(n * 4);
  for (int i = 0; i < n; i++)
    out += s;
  return out;
}

} // namespace ansi

// ─── terminal platform layer
// ──────────────────────────────────────────────────

namespace term {

#ifdef _WIN32
static HANDLE hOut, hIn;
static DWORD mOut, mIn;
#else
static termios saved;
#endif

void init() {
#ifdef _WIN32
  hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  hIn = GetStdHandle(STD_INPUT_HANDLE);
  GetConsoleMode(hOut, &mOut);
  GetConsoleMode(hIn, &mIn);
  // ENABLE_VIRTUAL_TERMINAL_PROCESSING lets us use ansi escape codes
  SetConsoleMode(hOut, mOut | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  SetConsoleMode(hIn, mIn & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#else
  tcgetattr(STDIN_FILENO, &saved);
  termios t = saved;
  t.c_lflag &= ~(ECHO | ICANON);
  t.c_cc[VMIN] = 0;
  t.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &t);
#endif
  std::cout << ansi::HIDE << ansi::CLR << std::flush;
}

void restore() {
#ifdef _WIN32
  SetConsoleMode(hOut, mOut);
  SetConsoleMode(hIn, mIn);
#else
  tcsetattr(STDIN_FILENO, TCSANOW, &saved);
#endif
  std::cout << ansi::SHOW << std::flush;
}

bool kbhit() {
#ifdef _WIN32
  return _kbhit() != 0;
#else
  char c;
  int n = read(STDIN_FILENO, &c, 1);
  if (n > 0) {
    ungetc(c, stdin);
    return true;
  }
  return false;
#endif
}

int getch() {
#ifdef _WIN32
  return _getch();
#else
  return getchar();
#endif
}

// two-tone beep on phase completion
void beep() {
#ifdef _WIN32
  Beep(880, 300);
  std::this_thread::sleep_for(std::chrono::milliseconds(80));
  Beep(1100, 400);
#else
  std::cout << '\a' << std::flush;
#endif
}

std::pair<int, int> size() {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO i;
  GetConsoleScreenBufferInfo(hOut, &i);
  return {i.srWindow.Bottom - i.srWindow.Top + 1,
          i.srWindow.Right - i.srWindow.Left + 1};
#else
  winsize ws{};
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  return {ws.ws_row, ws.ws_col};
#endif
}

} // namespace term

// ─── timer state machine
// ──────────────────────────────────────────────────────

enum class Phase { WORK, SHORT_BREAK, LONG_BREAK };

struct Timer {
  // default durations
  int work_s = 25 * 60;
  int short_s = 5 * 60;
  int long_s = 15 * 60;
  int cycle = 4; // work sessions before a long break

  Phase phase = Phase::WORK;
  int done = 0;    // completed work sessions total
  int elapsed = 0; // seconds into current phase
  bool paused = false;
  bool quit = false;

  int total() const {
    if (phase == Phase::SHORT_BREAK)
      return short_s;
    if (phase == Phase::LONG_BREAK)
      return long_s;
    return work_s;
  }
  int remaining() const { return total() - elapsed; }
  double progress() const { return std::min(1.0, (double)elapsed / total()); }

  void tick() {
    if (paused || quit)
      return;
    if (++elapsed >= total()) {
      term::beep();
      advance();
    }
  }

  void advance() {
    if (phase == Phase::WORK) {
      ++done;
      phase = (done % cycle == 0) ? Phase::LONG_BREAK : Phase::SHORT_BREAK;
    } else {
      phase = Phase::WORK;
    }
    elapsed = 0;
  }

  void reset() {
    elapsed = 0;
    paused = false;
  }
  void skip() {
    if (phase == Phase::WORK)
      ++done;
    advance();
  }

  const char *name() const {
    if (phase == Phase::SHORT_BREAK)
      return "short break";
    if (phase == Phase::LONG_BREAK)
      return "long break";
    return "focus";
  }
};

// ─── hourglass renderer
// ───────────────────────────────────────────────────────

namespace hg {

static const int HALF = 8;
static const int IMAX = 16;
static const int GWIDTH = IMAX + 2;           // 18
static const int GHEIGHT = 2 * HALF + 3;      // 19
static const int CAP = HALF * (IMAX + 2) / 2; // 72 sand cells per half

// inner width of top-half row r (1..HALF)
inline int iw(int r) { return IMAX - 2 * (r - 1); }

// sand cells in top-half row r given progress p in [0,1]
int sand_top(int r, double p) {
  double remaining = (1.0 - p) * CAP;
  double before = 0;
  for (int i = 1; i < r; i++)
    before += iw(i);
  int w = iw(r);
  double in_row = std::max(0.0, std::min((double)w, remaining - before));
  return (int)std::round(in_row);
}

// sand cells in bottom-half row r (r=1 near neck, r=HALF at floor) given
// progress p
int sand_bot(int r, double p) {
  double accumulated = p * CAP;
  double before = 0;
  for (int i = HALF; i > r; i--)
    before += iw(HALF + 1 - i);
  int w = iw(HALF + 1 - r);
  double in_row = std::max(0.0, std::min((double)w, accumulated - before));
  return (int)std::round(in_row);
}

// colour per phase
std::string sand_color(Phase ph) {
  if (ph == Phase::SHORT_BREAK)
    return ansi::rgb(config::active.short_b);
  if (ph == Phase::LONG_BREAK)
    return ansi::rgb(config::active.long_b);
  return ansi::rgb(config::active.work);
}

std::string glass_color() { return ansi::rgb(170, 160, 145); }
std::string grain_color() { return ansi::rgb(255, 230, 130); }

// draw hourglass into stream `out`; top-left at terminal (row0, col0)
void draw(std::ostream &out, const Timer &tmr, int row0, int col0, int frame) {
  double p = tmr.progress();
  bool run = !tmr.paused;
  auto sc = sand_color(tmr.phase);
  auto gc = glass_color();

  auto put = [&](int r, const std::string &line) {
    out << ansi::mv(row0 + r, col0) << line << ansi::RST << ansi::EL;
  };

  // ── top border ────────────────────────────────────────────────────────
  put(0, gc + "\u2554" + ansi::rep("\u2550", IMAX) + "\u2557");

  // ── top half ──────────────────────────────────────────────────────────
  for (int r = 1; r <= HALF; r++) {
    int w = iw(r);
    int ind = r - 1;
    int sand = sand_top(r, p);
    std::string line;

    if (r == 1) {
      line += gc + "\u2551" + sc;
      for (int i = 0; i < w; i++)
        line += (i < sand) ? "\u2593" : " ";
      line += gc + "\u2551";
    } else {
      line += std::string(ind, ' ');
      line += gc + "\\";
      line += sc;
      for (int i = 0; i < w; i++)
        line += (i < sand) ? "\u2593" : " ";
      line += gc + "/";
      line += std::string(ind, ' ');
    }
    put(r, line);
  }

  // ── pinch row ─────────────────────────────────────────────────────────
  {
    bool show_grain = run && (frame / 3 % 2 == 0);
    std::string g = show_grain ? (grain_color() + "\xC2\xB7" + gc) : " ";
    put(HALF + 1, std::string(HALF, ' ') + gc + ">" + g + "<" +
                      std::string(HALF - 1, ' '));
  }

  // ── bottom half ───────────────────────────────────────────────────────
  for (int r = 1; r <= HALF; r++) {
    int hrow = HALF + 1 + r;
    int w = iw(HALF + 1 - r);
    int ind = HALF - r;
    int sand = sand_bot(r, p);
    int le = (w > sand) ? (w - sand) / 2 : 0;

    std::string line;
    if (r == HALF) {
      line += gc + "\u2551" + sc;
      for (int i = 0; i < w; i++) {
        bool filled = (sand == w) || (sand > 0 && i >= le && i < le + sand);
        line += filled ? "\u2593" : " ";
      }
      line += gc + "\u2551";
    } else {
      line += std::string(ind, ' ');
      line += gc + "/";
      line += sc;
      for (int i = 0; i < w; i++) {
        bool filled = (sand == w) || (sand > 0 && i >= le && i < le + sand);
        line += filled ? "\u2593" : " ";
      }
      line += gc + "\\";
      line += std::string(ind, ' ');
    }
    put(hrow, line);
  }

  // ── bottom border ─────────────────────────────────────────────────────
  put(2 * HALF + 2, gc + "\u255a" + ansi::rep("\u2550", IMAX) + "\u255d");
}

} // namespace hg

// ─── big digit glyphs ────────────────────────────────────────────────────────

static const char *DG[11][5] = {
    // 0
    {"\u2588\u2588\u2588\u2588\u2588\u2588", "\u2588\u2588  \u2588\u2588",
     "\u2588\u2588  \u2588\u2588", "\u2588\u2588  \u2588\u2588",
     "\u2588\u2588\u2588\u2588\u2588\u2588"},
    // 1
    {"  \u2588\u2588  ", "  \u2588\u2588  ", "  \u2588\u2588  ",
     "  \u2588\u2588  ", "  \u2588\u2588  "},
    // 2
    {"\u2588\u2588\u2588\u2588\u2588\u2588", "    \u2588\u2588",
     "\u2588\u2588\u2588\u2588\u2588\u2588", "\u2588\u2588    ",
     "\u2588\u2588\u2588\u2588\u2588\u2588"},
    // 3
    {"\u2588\u2588\u2588\u2588\u2588\u2588", "    \u2588\u2588",
     "\u2588\u2588\u2588\u2588\u2588\u2588", "    \u2588\u2588",
     "\u2588\u2588\u2588\u2588\u2588\u2588"},
    // 4
    {"\u2588\u2588  \u2588\u2588", "\u2588\u2588  \u2588\u2588",
     "\u2588\u2588\u2588\u2588\u2588\u2588", "    \u2588\u2588",
     "    \u2588\u2588"},
    // 5
    {"\u2588\u2588\u2588\u2588\u2588\u2588", "\u2588\u2588    ",
     "\u2588\u2588\u2588\u2588\u2588\u2588", "    \u2588\u2588",
     "\u2588\u2588\u2588\u2588\u2588\u2588"},
    // 6
    {"\u2588\u2588\u2588\u2588\u2588\u2588", "\u2588\u2588    ",
     "\u2588\u2588\u2588\u2588\u2588\u2588", "\u2588\u2588  \u2588\u2588",
     "\u2588\u2588\u2588\u2588\u2588\u2588"},
    // 7
    {"\u2588\u2588\u2588\u2588\u2588\u2588", "    \u2588\u2588",
     "  \u2588\u2588\u2588\u2588", "    \u2588\u2588", "    \u2588\u2588"},
    // 8
    {"\u2588\u2588\u2588\u2588\u2588\u2588", "\u2588\u2588  \u2588\u2588",
     "\u2588\u2588\u2588\u2588\u2588\u2588", "\u2588\u2588  \u2588\u2588",
     "\u2588\u2588\u2588\u2588\u2588\u2588"},
    // 9
    {"\u2588\u2588\u2588\u2588\u2588\u2588", "\u2588\u2588  \u2588\u2588",
     "\u2588\u2588\u2588\u2588\u2588\u2588", "    \u2588\u2588",
     "\u2588\u2588\u2588\u2588\u2588\u2588"},
    // : (index 10)
    {"  ", "\u25aa\u25aa", "  ", "\u25aa\u25aa", "  "},
};

void draw_clock(std::ostream &out, int secs, const std::string &color, int row0,
                int col0) {
  int mm = secs / 60;
  int ss = secs % 60;
  int d[5] = {mm / 10, mm % 10, 10, ss / 10, ss % 10};

  for (int row = 0; row < 5; row++) {
    out << ansi::mv(row0 + row, col0) << color;
    for (int di = 0; di < 5; di++) {
      out << DG[d[di]][row];
      if (di < 4)
        out << " ";
    }
    out << ansi::RST << ansi::EL;
  }
}

// ─── wall clock helper ───────────────────────────────────────────────────────

std::string wall_clock() {
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  struct tm *tm_info = std::localtime(&t);
  char buf[32];
  strftime(buf, sizeof buf, "%a %d %b  %H:%M:%S", tm_info);
  for (char &c : buf)
    if (c >= 'A' && c <= 'Z')
      c = (char)(c + 32);
  return buf;
}

// ─── developer splash / boot screen ──────────────────────────────────────────

void splash(int /*rows*/, int cols) {
  const int BOX_W = 40;
  const int BOX_H = 13;
  int bc = (cols - BOX_W) / 2;
  int br = 4;

  std::cout << ansi::CLR;

  auto gc = ansi::rgb(170, 160, 145);
  auto dc = ansi::rgb(90, 88, 80);
  auto gr = ansi::rgb(70, 210, 130);
  auto am = ansi::rgb(220, 140, 40);

  std::cout << ansi::mv(br, bc) << dc << "\u250c"
            << ansi::rep("\u2500", BOX_W - 2) << "\u2510" << ansi::RST;
  for (int r = 1; r < BOX_H - 1; r++) {
    std::cout << ansi::mv(br + r, bc) << dc << "\u2502"
              << std::string(BOX_W - 2, ' ') << "\u2502" << ansi::RST;
  }
  std::cout << ansi::mv(br + BOX_H - 1, bc) << dc << "\u2514"
            << ansi::rep("\u2500", BOX_W - 2) << "\u2518" << ansi::RST;

  std::cout << ansi::mv(br + 2, bc + 4) << am << ansi::BOLD << "pomodoro"
            << ansi::RST;
  std::cout << ansi::mv(br + 3, bc + 4) << gc << "terminal focus timer  v1.0"
            << ansi::RST;

  std::cout.flush();

  struct Entry {
    const char *label;
    int delay_ms;
  };
  Entry entries[] = {
      {"terminal detection   ", 160},
      {"utf-8 / ansi mode    ", 90},
      {"hourglass renderer   ", 130},
      {"config defaults      ", 70},
  };

  int row = br + 5;
  for (auto &e : entries) {
    std::cout << ansi::mv(row, bc + 4) << dc << e.label << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(e.delay_ms));
    std::cout << "  " << gr << "[ok]" << ansi::RST << std::flush;
    ++row;
  }
  std::cout << ansi::mv(br + BOX_H - 2, bc + 4) << gc << "Select theme: " << am
            << "[1] Amber  " << ansi::rgb(0, 255, 255) << "[2] Tron"
            << ansi::RST << std::flush;

  bool selected = false;
  while (!selected) {
    if (term::kbhit()) {
      switch (term::getch()) {
      case '1':
        config::active = config::THEME_AMBER;
        selected = true;
        break;
      case '2':
        config::active = config::THEME_TRON;
        selected = true;
        break;
      case '3':
        config::active = config::STEVAN_MODE;
        selected = true;
        break;
      case '\n':
      case '\r':
      case ' ':
        selected = true; // Default to current/Amber
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
  }
  std::cout << ansi::CLR << std::flush;
}

// ─── main render ─────────────────────────────────────────────────────────────

void render(std::ostream &out, const Timer &tmr, int /*rows*/, int cols,
            int frame) {
  if (cols < 60) {
    out << ansi::mv(1, 1) << ansi::rgb(255, 80, 80)
        << "terminal too small — need >= 60 cols" << ansi::RST;
    out << std::flush;
    return;
  }

  auto sand_c = hg::sand_color(tmr.phase);
  auto gray_c = ansi::rgb(130, 125, 115);
  auto dim_c = ansi::rgb(90, 88, 80);
  auto amber_c = ansi::rgb(220, 140, 40);

  const int hg_row = 3;
  const int hg_col = 3;
  const int rp_col = hg_col + hg::GWIDTH + 3;

  {
    std::string wc = wall_clock();
    int wc_col = cols - (int)wc.size() - 1;

    out << ansi::mv(1, 2) << amber_c << ansi::BOLD << "pomodoro" << ansi::RST
        << gray_c << "  \xe2\x80\x94  terminal focus timer" << ansi::RST
        << ansi::EL;

    out << ansi::mv(1, wc_col) << gray_c << wc << ansi::RST;
    out << ansi::mv(2, 1) << dim_c << ansi::rep("\u2500", cols - 1) << ansi::RST
        << ansi::EL;
  }

  hg::draw(out, tmr, hg_row, hg_col, frame);

  out << ansi::mv(hg_row, rp_col) << dim_c << "countdown" << ansi::RST
      << ansi::EL;

  draw_clock(out, tmr.remaining(), sand_c, hg_row + 1, rp_col);

  {
    int pr = hg_row + 7;
    std::string ph = std::string("  ") + tmr.name() + "  ";
    out << ansi::mv(pr, rp_col) << sand_c << ansi::BOLD << ph << ansi::RST
        << ansi::EL;
  }

  {
    int dr = hg_row + 8;
    out << ansi::mv(dr, rp_col);
    int in_cyc = tmr.done % tmr.cycle;
    bool full = (tmr.done > 0 && tmr.done % tmr.cycle == 0);
    for (int i = 0; i < tmr.cycle; i++) {
      bool done = full || (i < in_cyc);
      bool current = !done && (i == in_cyc) && (tmr.phase == Phase::WORK);
      if (done)
        out << sand_c << "\u25cf ";
      else if (current)
        out << sand_c << "\u25ce ";
      else
        out << dim_c << "\u25cb ";
    }
    out << gray_c << "  #" << (tmr.done + 1) << ansi::RST << ansi::EL;
  }

  {
    int par = hg_row + 10;
    out << ansi::mv(par, rp_col);
    if (tmr.paused)
      out << ansi::rgb(240, 210, 50) << "  paused" << ansi::RST;
    out << ansi::EL;
  }

  {
    int cr = hg_row + 12;
    out << ansi::mv(cr, rp_col) << dim_c << "\u2500 controls" << ansi::RST
        << ansi::EL;

    struct Bind {
      const char *key;
      const char *desc;
    };
    Bind binds[] = {
        {"spc", "pause / resume"},
        {"s  ", "skip phase"},
        {"r  ", "reset timer"},
        {"q  ", "quit"},
    };

    auto kc = ansi::rgb(200, 180, 80);
    for (int i = 0; i < 4; i++) {
      out << ansi::mv(cr + 1 + i, rp_col) << kc << "[" << binds[i].key << "]"
          << ansi::RST << gray_c << "  " << binds[i].desc << ansi::RST
          << ansi::EL;
    }
  }

  out << std::flush;
}

// ─── entry point ─────────────────────────────────────────────────────────────

static volatile sig_atomic_t g_quit = 0;
void on_signal(int) { g_quit = 1; }

int main() {
  signal(SIGINT, on_signal);
  signal(SIGTERM, on_signal);

  term::init();

  auto [rows, cols] = term::size();
  splash(rows, cols);

  Timer tmr;
  int frame = 0;
  auto last = std::chrono::steady_clock::now();

  std::ostringstream buf;

  while (!g_quit && !tmr.quit) {
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last)
            .count() >= 1000) {
      tmr.tick();
      last = now;
    }

    auto [r, c] = term::size();
    buf.str("");
    buf.clear();
    buf << ansi::mv(1, 1);
    render(buf, tmr, r, c, frame++);
    std::cout << buf.str();

    while (term::kbhit()) {
      switch (std::tolower(term::getch())) {
      case ' ':
        tmr.paused = !tmr.paused;
        break;
      case 's':
        tmr.skip();
        break;
      case 'r':
        tmr.reset();
        break;
      case 'q':
        tmr.quit = true;
        break;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::cout << ansi::CLR << ansi::RST << std::flush;
  term::restore();
  return 0;
}
