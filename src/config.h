
#ifndef POMODORO_CONFIG_H
#define POMODORO_CONFIG_H

#include <cstdint>

/**
 * @file config.h
 * @brief Configuration definitions and theme palette settings for Pomodoro timer.
 */

namespace config {

/// RGB color structure (0-255 per channel).
struct Color {
  int r, g, b;
};

/// Theme color set containing active colors for work, short break, and long break phases.
struct Theme {
  Color work, short_b, long_b;
};

// Preset theme colors (RGB values)
constexpr Theme THEME_AMBER = {{220, 140, 40}, {70, 210, 130}, {80, 160, 240}};
constexpr Theme THEME_TRON  = {{0, 255, 255},  {0, 150, 255},  {0, 50, 255}};

// Custom theme preset
constexpr Theme STEVAN_MODE = {{0, 255, 255},  {0, 150, 255},  {0, 50, 255}};

// Mutable active color theme (default is Amber).
// Marked inline so it is shared across all translation units.
inline Theme active = THEME_AMBER;

// Constants for glass frame and sand grain details
constexpr Color GLASS = {170, 160, 145};
constexpr Color GRAIN = {255, 230, 130};

} // namespace config

#endif // POMODORO_CONFIG_H

