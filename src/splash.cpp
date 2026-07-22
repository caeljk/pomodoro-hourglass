#include "splash.h"
#include "ansi.h"
#include "config.h"
#include <chrono>
#include <iostream>
#include <thread>

/**
 * @file splash.cpp
 * @brief Implementation of startup splash screen with theme selector.
 */

void splash(int rows, int cols) {
  const int BOX_W = 40;
  const int BOX_H = 13;
  int bc = (cols - BOX_W) / 2;
  int br = 4;

  std::cout << ansi::CLR;

  auto gc = ansi::rgb(170, 160, 145);
  auto dc = ansi::rgb(90, 88, 80);
  auto gr = ansi::rgb(70, 210, 130);
  auto am = ansi::rgb(220, 140, 40);

  // ── Draw splash border frame box ─────────────────────────────────────────
  std::cout << ansi::mv(br, bc) << dc << "\u250c"
            << ansi::rep("\u2500", BOX_W - 2) << "\u2510" << ansi::RST;
  for (int r = 1; r < BOX_H - 1; r++) {
    std::cout << ansi::mv(br + r, bc) << dc << "\u2502"
              << std::string(BOX_W - 2, ' ') << "\u2502" << ansi::RST;
  }
  std::cout << ansi::mv(br + BOX_H - 1, bc) << dc << "\u2514"
            << ansi::rep("\u2500", BOX_W - 2) << "\u2518" << ansi::RST;

  // ── Title & version string ───────────────────────────────────────────────
  std::cout << ansi::mv(br + 2, bc + 4) << am << ansi::BOLD << "pomodoro"
            << ansi::RST;
  std::cout << ansi::mv(br + 3, bc + 4) << gc << "terminal focus timer  v1.0"
            << ansi::RST;

  std::cout.flush();

  // ── Boot checklist sequence animation ──────────────────────────────────
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

  // ── Theme Selection Prompt ──────────────────────────────────────────────
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
        selected = true; // Default to Amber/current active theme
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
  }
  std::cout << ansi::CLR << std::flush;
}

