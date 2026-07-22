#ifndef POMODORO_ANSI_H
#define POMODORO_ANSI_H

#include "config.h"
#include <cstdio>
#include <string>

/**
 * @file ansi.h
 * @brief ANSI escape code helpers for terminal manipulation and 24-bit RGB coloring.
 */

namespace ansi {

// Position cursor at row r, col c (1-indexed) using ANSI CSI escape code
inline std::string mv(int r, int c) {
  char b[24];
  snprintf(b, sizeof b, "\033[%d;%dH", r, c);
  return b;
}

// 24-bit foreground colour (raw integer RGB interface)
inline std::string rgb(int r, int g, int b) {
  char buf[32];
  snprintf(buf, sizeof buf, "\033[38;2;%d;%d;%dm", r, g, b);
  return buf;
}

// 24-bit foreground colour (struct interface using config::Color)
inline std::string rgb(config::Color c) { return rgb(c.r, c.g, c.b); }

// ANSI Formatting Constants
constexpr const char *RST     = "\033[0m";       // Reset all attributes
constexpr const char *BOLD    = "\033[1m";       // Bold text
constexpr const char *DIM     = "\033[2m";       // Dimmed / faint text
constexpr const char *CLR     = "\033[2J\033[H"; // Clear screen & move cursor home
constexpr const char *HIDE    = "\033[?25l";     // Hide terminal cursor
constexpr const char *SHOW    = "\033[?25h";     // Show terminal cursor
constexpr const char *EL      = "\033[K";        // Erase rest of line from cursor
constexpr const char *ALT_ON  = "\033[?1049h";   // Enable alternate screen buffer
constexpr const char *ALT_OFF = "\033[?1049l";   // Restore main screen buffer

// Repeat a UTF-8 string n times (handles multi-byte glyphs correctly)
inline std::string rep(const char *s, int n) {
  std::string out;
  out.reserve(n * 4);
  for (int i = 0; i < n; i++)
    out += s;
  return out;
}

} // namespace ansi

#endif // POMODORO_ANSI_H

