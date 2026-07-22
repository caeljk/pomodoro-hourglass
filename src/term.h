#ifndef POMODORO_TERM_H
#define POMODORO_TERM_H

#include "ansi.h"
#include <iostream>
#include <thread>
#include <utility>

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

/**
 * @file term.h
 * @brief Cross-platform raw terminal control, non-blocking keyboard input, and window size querying.
 */

namespace term {

#ifdef _WIN32
inline HANDLE hOut, hIn;
inline DWORD mOut, mIn;
#else
inline termios saved;
#endif

/**
 * @brief Initialize terminal raw mode and enable ANSI sequence processing.
 */
inline void init() {
#ifdef _WIN32
  hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  hIn = GetStdHandle(STD_INPUT_HANDLE);
  GetConsoleMode(hOut, &mOut);
  GetConsoleMode(hIn, &mIn);
  // ENABLE_VIRTUAL_TERMINAL_PROCESSING enables ANSI escape sequence parsing on Windows
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

/**
 * @brief Restore terminal settings to original state upon exit.
 */
inline void restore() {
#ifdef _WIN32
  SetConsoleMode(hOut, mOut);
  SetConsoleMode(hIn, mIn);
#else
  tcsetattr(STDIN_FILENO, TCSANOW, &saved);
#endif
  std::cout << ansi::SHOW << std::flush;
}

/**
 * @brief Check if a key has been pressed without blocking.
 * @return true if input is available in the keyboard buffer.
 */
inline bool kbhit() {
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

/**
 * @brief Read a single character from terminal input without echo.
 */
inline int getch() {
#ifdef _WIN32
  return _getch();
#else
  return getchar();
#endif
}

/**
 * @brief Emit audio signal notification on timer phase transitions.
 */
inline void beep() {
#ifdef _WIN32
  Beep(880, 300);
  std::this_thread::sleep_for(std::chrono::milliseconds(80));
  Beep(1100, 400);
#else
  std::cout << '\a' << std::flush;
#endif
}

/**
 * @brief Query current terminal window dimensions.
 * @return std::pair<int, int> containing {rows, columns}.
 */
inline std::pair<int, int> size() {
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

#endif // POMODORO_TERM_H

