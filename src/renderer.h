#ifndef POMODORO_RENDERER_H
#define POMODORO_RENDERER_H

#include "config.h"
#include "timer.h"
#include <ostream>
#include <string>

/**
 * @file renderer.h
 * @brief UI rendering engine for ASCII hourglass animation and big digit timer display.
 */

namespace hg {

// Geometry constants for the hourglass drawing
static const int HALF = 8;
static const int IMAX = 16;
static const int GWIDTH = IMAX + 2;           // 18 columns wide
static const int GHEIGHT = 2 * HALF + 3;      // 19 rows tall
static const int CAP = HALF * (IMAX + 2) / 2; // 72 total sand cells per half

/// Inner width of top-half row r (1..HALF)
inline int iw(int r) { return IMAX - 2 * (r - 1); }

/// Calculate number of sand cells in top-half row r for progress p in [0, 1]
int sand_top(int r, double p);

/// Calculate number of sand cells in bottom-half row r for progress p
int sand_bot(int r, double p);

/// Color sequence string for active phase
std::string sand_color(Phase ph);

/// Color sequence string for glass boundary
std::string glass_color();

/// Color sequence string for falling sand grain
std::string grain_color();

/// Draw hourglass graphics into stream output at specified terminal coordinates
void draw(std::ostream &out, const Timer &tmr, int row0, int col0, int frame);

} // namespace hg

// ─── Big Digit Glyphs ────────────────────────────────────────────────────────
// 5-line height glyph representations for digits '0'-'9' and ':' (index 10)
inline constexpr const char *DG[11][5] = {
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

/// Render big 5-line digital clock showing mm:ss
void draw_clock(std::ostream &out, int secs, const std::string &color, int row0,
                int col0);

/// Helper returning formatted current local system time string
std::string wall_clock();

/// Render full Pomodoro interface layout to stream buffer
void render(std::ostream &out, const Timer &tmr, int rows, int cols,
            int frame);

#endif // POMODORO_RENDERER_H

