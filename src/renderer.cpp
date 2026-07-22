#include "renderer.h"
#include "ansi.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
#include <ostream>

/**
 * @file renderer.cpp
 * @brief Implementation of ASCII hourglass drawing and full UI layout renderer.
 */

namespace hg {

// Calculate remaining sand cells in row r of top half based on progress p [0, 1]
int sand_top(int r, double p) {
  double remaining = (1.0 - p) * CAP;
  double before = 0;
  for (int i = 1; i < r; i++)
    before += iw(i);
  int w = iw(r);
  double in_row = std::max(0.0, std::min((double)w, remaining - before));
  return (int)std::round(in_row);
}

// Calculate accumulated sand cells in row r of bottom half based on progress p
int sand_bot(int r, double p) {
  double accumulated = p * CAP;
  double before = 0;
  for (int i = HALF; i > r; i--)
    before += iw(HALF + 1 - i);
  int w = iw(HALF + 1 - r);
  double in_row = std::max(0.0, std::min((double)w, accumulated - before));
  return (int)std::round(in_row);
}

// Select ANSI sand color according to active session phase
std::string sand_color(Phase ph) {
  if (ph == Phase::SHORT_BREAK)
    return ansi::rgb(config::active.short_b);
  if (ph == Phase::LONG_BREAK)
    return ansi::rgb(config::active.long_b);
  return ansi::rgb(config::active.work);
}

// Glass frame color
std::string glass_color() { return ansi::rgb(170, 160, 145); }

// Falling grain animation color
std::string grain_color() { return ansi::rgb(255, 230, 130); }

// Draw animated ASCII hourglass at specified (row0, col0) terminal position
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

  // ── pinch row (animated falling sand grain) ─────────────────────────────
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

// Draw 5-line height big digit timer display
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

// Return formatted wall clock string (e.g. "wed 22 jul  19:49:08")
std::string wall_clock() {
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  struct tm *tm_info = std::localtime(&t);
  char buf[32];
  strftime(buf, sizeof buf, "%a %d %b  %H:%M:%S", tm_info);
  std::string s = buf;
  for (char &c : s)
    if (c >= 'A' && c <= 'Z')
      c = (char)(c + 32);
  return s;
}

// Render complete Pomodoro terminal interface
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

  // Header & Wall clock display
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

  // Draw hourglass animation
  hg::draw(out, tmr, hg_row, hg_col, frame);

  // Label & Digital clock display
  out << ansi::mv(hg_row, rp_col) << dim_c << "countdown" << ansi::RST
      << ansi::EL;

  draw_clock(out, tmr.remaining(), sand_c, hg_row + 1, rp_col);

  // Active phase name badge
  {
    int pr = hg_row + 7;
    std::string ph = std::string("  ") + tmr.name() + "  ";
    out << ansi::mv(pr, rp_col) << sand_c << ansi::BOLD << ph << ansi::RST
        << ansi::EL;
  }

  // Cycle progress session dots (● completed, ◎ active, ○ upcoming)
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

  // Paused status indicator
  {
    int par = hg_row + 10;
    out << ansi::mv(par, rp_col);
    if (tmr.paused)
      out << ansi::rgb(240, 210, 50) << "  paused" << ansi::RST;
    out << ansi::EL;
  }

  // Controls legend block
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

