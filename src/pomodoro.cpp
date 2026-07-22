#include "ansi.h"
#include "renderer.h"
#include "splash.h"
#include "term.h"
#include "timer.h"
#include <algorithm>
#include <csignal>
#include <iostream>
#include <sstream>
#include <thread>

/**
 * @file pomodoro.cpp
 * @brief Entry point and main event loop for the terminal Pomodoro timer.
 */

// ─── Signal handling & entry point ──────────────────────────────────────────

static volatile sig_atomic_t g_quit = 0;
void on_signal(int) { g_quit = 1; }

int main() {
  // Register OS interrupt and termination signal handlers
  signal(SIGINT, on_signal);
  signal(SIGTERM, on_signal);

  // Initialize raw terminal mode and ANSI sequence support
  term::init();

  // Query terminal window dimensions and show welcome splash screen
  auto [rows, cols] = term::size();
  splash(rows, cols);

  Timer tmr;
  int frame = 0;
  auto last = std::chrono::steady_clock::now();

  std::ostringstream buf;

  // ─── Main Render & Event Loop ──────────────────────────────────────────────
  while (!g_quit && !tmr.quit) {
    auto now = std::chrono::steady_clock::now();
    // Advance timer state every 1000 milliseconds (1 second)
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last)
            .count() >= 1000) {
      tmr.tick();
      last = now;
    }

    // Query terminal size each frame for responsive resizing
    auto [r, c] = term::size();

    // Render frame to stringstream buffer to avoid screen flickering
    buf.str("");
    buf.clear();
    buf << ansi::mv(1, 1);
    render(buf, tmr, r, c, frame++);
    std::cout << buf.str();

    // Process all pending keyboard input without blocking
    while (term::kbhit()) {
      switch (std::tolower(term::getch())) {
      case ' ': // Space: Pause or resume timer
        tmr.paused = !tmr.paused;
        break;
      case 's': // 's': Skip current phase
        tmr.skip();
        break;
      case 'r': // 'r': Reset current phase countdown
        tmr.reset();
        break;
      case 'q': // 'q': Quit application
        tmr.quit = true;
        break;
      }
    }

    // Sleep for 100ms per frame (~10 FPS render cycle)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // ─── Application Cleanup ───────────────────────────────────────────────────
  std::cout << ansi::CLR << ansi::RST << std::flush;
  term::restore();
  return 0;
}

