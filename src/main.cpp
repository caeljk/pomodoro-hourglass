// pomodoro.cpp
// terminal pomodoro timer — hourglass + big-digit clock side by side
//
// keys:
//   space  - pause / resume
//   s      - skip to next phase
//   r      - reset current timer
//   q      - quit

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <cmath>
#include <ctime>
#include <csignal>
#include <algorithm>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <conio.h>
#else
#  include <termios.h>
#  include <unistd.h>
#  include <sys/ioctl.h>
#endif

namespace ansi {
inline std::string mv(int r, int c) {
    char b[24];
    snprintf(b, sizeof b, "\033[%d;%dH", r, c);
    return b;
}
inline std::string rgb(int r, int g, int b) {
    char buf[32];
    snprintf(buf, sizeof buf, "\033[38;2;%d;%d;%dm", r, g, b);
    return buf;
}
constexpr const char* RST  = "\033[0m";
constexpr const char* BOLD = "\033[1m";
constexpr const char* DIM  = "\033[2m";
constexpr const char* CLR  = "\033[2J\033[H";
constexpr const char* HIDE = "\033[?25l";
constexpr const char* SHOW = "\033[?25h";
constexpr const char* EL   = "\033[K";
inline std::string rep(const char* s, int n) {
    std::string out;
    out.reserve(n * 4);
    for (int i = 0; i < n; i++) out += s;
    return out;
}
}

namespace term {
#ifdef _WIN32
    static HANDLE hOut, hIn;
    static DWORD  mOut, mIn;
#else
    static termios saved;
#endif

void init() {
#ifdef _WIN32
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hIn  = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hOut, &mOut);
    GetConsoleMode(hIn,  &mIn);
    SetConsoleMode(hOut, mOut | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleMode(hIn,  mIn  & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
    tcgetattr(STDIN_FILENO, &saved);
    termios t = saved;
    t.c_lflag &= ~(ECHO | ICANON);
    t.c_cc[VMIN]  = 0;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
#endif
    std::cout << ansi::HIDE << ansi::CLR << std::flush;
}

void restore() {
#ifdef _WIN32
    SetConsoleMode(hOut, mOut);
    SetConsoleMode(hIn,  mIn);
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
    if (n > 0) { ungetc(c, stdin); return true; }
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
    return { i.srWindow.Bottom - i.srWindow.Top + 1,
             i.srWindow.Right  - i.srWindow.Left + 1 };
#else
    winsize ws{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    return { ws.ws_row, ws.ws_col };
#endif
}
}

enum class Phase { WORK, SHORT_BREAK, LONG_BREAK };

struct Timer {
    int work_s  = 25 * 60;
    int short_s =  5 * 60;
    int long_s  = 15 * 60;
    int cycle   = 4;

    Phase phase   = Phase::WORK;
    int   done    = 0;
    int   elapsed = 0;
    bool  paused  = false;
    bool  quit    = false;

    int total() const {
        if (phase == Phase::SHORT_BREAK) return short_s;
        if (phase == Phase::LONG_BREAK)  return long_s;
        return work_s;
    }
    int remaining() const { return total() - elapsed; }
    double progress() const { return std::min(1.0, (double)elapsed / total()); }

    void tick() {
        if (paused || quit) return;
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

    void reset() { elapsed = 0; paused = false; }
    void skip()  { if (phase == Phase::WORK) ++done; advance(); }

    const char* name() const {
        if (phase == Phase::SHORT_BREAK) return "short break";
        if (phase == Phase::LONG_BREAK)  return "long break";
        return "focus";
    }
};

namespace hg {
static const int HALF    = 8;
static const int IMAX    = 16;
static const int GWIDTH  = IMAX + 2;
static const int GHEIGHT = 2 * HALF + 3;
static const int CAP     = HALF * (IMAX + 2) / 2;

inline int iw(int r) { return IMAX - 2 * (r - 1); }

int sand_top(int r, double p) {
    double remaining = (1.0 - p) * CAP;
    double before = 0;
    for (int i = 1; i < r; i++) before += iw(i);
    int w = iw(r);
    double in_row = std::max(0.0, std::min((double)w, remaining - before));
    return (int)std::round(in_row);
}

int sand_bot(int r, double p) {
    double accumulated = p * CAP;
    double before = 0;
    for (int i = HALF; i > r; i--) before += iw(HALF + 1 - i);
    int w = iw(HALF + 1 - r);
    double in_row = std::max(0.0, std::min((double)w, accumulated - before));
    return (int)std::round(in_row);
}

std::string sand_color(Phase ph) {
    if (ph == Phase::SHORT_BREAK) return ansi::rgb(70,  210, 130);
    if (ph == Phase::LONG_BREAK)  return ansi::rgb(80,  160, 240);
    return                               ansi::rgb(220, 140,  40);
}

std::string glass_color() { return ansi::rgb(170, 160, 145); }
std::string grain_color() { return ansi::rgb(255, 230, 130); }

void draw(std::ostream& out, const Timer& tmr, int row0, int col0, int frame) {
    double p   = tmr.progress();
    bool   run = !tmr.paused;
    auto   sc  = sand_color(tmr.phase);
    auto   gc  = glass_color();

    auto put = [&](int r, const std::string& line) {
        out << ansi::mv(row0 + r, col0) << line << ansi::RST << ansi::EL;
    };

    put(0, gc + "\u2554" + ansi::rep("\u2550", IMAX) + "\u2557");

    for (int r = 1; r <= HALF; r++) {
        int w    = iw(r);
        int ind  = r - 1;
        int sand = sand_top(r, p);
        std::string line;

        if (r == 1) {
            line += gc + "\u2551" + sc;
            for (int i = 0; i < w; i++) line += (i < sand) ? "\u2593" : " ";
            line += gc + "\u2551";
        } else {
            line += std::string(ind, ' ');
            line += gc + "\\";
            line += sc;
            for (int i = 0; i < w; i++) line += (i < sand) ? "\u2593" : " ";
            line += gc + "/";
            line += std::string(ind, ' ');
        }
        put(r, line);
    }

    {
        bool show_grain = run && (frame / 3 % 2 == 0);
        std::string g   = show_grain ? (grain_color() + "\xC2\xB7" + gc) : " ";
        put(HALF + 1, std::string(HALF, ' ') + gc + ">" + g + "<" + std::string(HALF - 1, ' '));
    }

    for (int r = 1; r <= HALF; r++) {
        int hrow = HALF + 1 + r;
        int w    = iw(HALF + 1 - r);
        int ind  = HALF - r;
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

    put(2 * HALF + 2, gc + "\u255a" + ansi::rep("\u2550", IMAX) + "\u255d");
}
}
// dev build stage 1