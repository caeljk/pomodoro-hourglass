# Pomodoro Hourglass Focus Timer (Modular Edition)

A modern, highly modular C++20 terminal-based Pomodoro focus timer featuring a dynamic ASCII hourglass animation, big-digit digital clock display, live system wall-clock, custom color themes, and cross-platform non-blocking terminal controls.

---

## 🏗️ Project Architecture & Structure

The codebase is organized into cleanly separated modules following the Single Responsibility Principle:

```
src/modular-version/
├── ansi.h          # ANSI escape codes (cursor movement, 24-bit RGB truecolor, screen clearing)
├── config.h        # Configuration, Color structs, theme presets (Amber, Tron, Stevan Mode)
├── term.h          # Platform terminal layer (raw mode, non-blocking kbhit, getch, window size, beep)
├── timer.h         # Timer state machine definition (Phase enum, Timer struct)
├── timer.cpp       # Timer state machine implementation (tick, advance, reset, skip, duration)
├── renderer.h      # UI rendering interface (hourglass math, 5-line big digit glyphs)
├── renderer.cpp    # UI layout engine (hourglass drawing, digital clock, wall clock, status bar)
├── splash.h        # Boot splash screen interface
├── splash.cpp      # Boot splash screen sequence & interactive theme selection prompt
└── pomodoro.cpp    # Application entry point (main), signal handling, 10 FPS event loop
```

---

## 📋 Module Responsibilities

| File | Responsibilities |
|---|---|
| **`config.h`** | Defines `Color` (RGB) and `Theme` structures. Stores theme presets (`THEME_AMBER`, `THEME_TRON`, `STEVAN_MODE`) and global active theme (`inline Theme active`). |
| **`ansi.h`** | Header-only ANSI helper functions for 24-bit RGB colors (`ansi::rgb`), cursor positioning (`ansi::mv`), screen clearing, and UTF-8 string repetition. |
| **`term.h`** | Cross-platform terminal abstraction for Windows (`conio.h`, `windows.h`) and POSIX (`termios`, `ioctl`, `unistd`). Enables raw input mode, non-blocking `kbhit()`, unbuffered `getch()`, audio chime `beep()`, and window size detection. |
| **`timer.h` / `timer.cpp`** | Core Pomodoro state machine tracking focus sessions (25 min), short breaks (5 min), and long breaks (15 min). Manages cycle counting, progress calculation, and phase transitions. |
| **`renderer.h` / `renderer.cpp`** | Full UI rendering engine. Draws top/bottom half sand levels for the ASCII hourglass, renders 5-line height big digit clocks (`DG`), prints system wall-clock time, and draws session cycle indicators. |
| **`splash.h` / `splash.cpp`** | Interactive startup sequence displaying animated boot checklist items and theme selection prompt. |
| **`pomodoro.cpp`** | Main entry point (`main()`), OS signal handling (`SIGINT`, `SIGTERM`), double-buffered terminal rendering to `std::ostringstream`, and key event dispatch loop. |

---

## ⚡ Build Instructions

### Prerequisites
- C++20 compliant compiler (`g++` 10+, `clang++` 12+, or MSVC 2019+)

### Direct Compilation with G++
Run the following command from the `src/modular-version` directory:

```bash
g++ -std=c++20 -O3 -Wall \
    pomodoro.cpp timer.cpp renderer.cpp splash.cpp \
    -o pomodoro
```

### Running the Application

- **Linux / macOS / Git Bash / PowerShell**:
  ```bash
  ./pomodoro
  ```

---

## 🎮 Controls & Shortcuts

| Key | Description |
|---|---|
| `<Space>` | Pause / Resume timer countdown |
| `s` | Skip current phase (advance to next phase) |
| `r` | Reset timer countdown for current phase |
| `q` | Quit application and restore terminal settings |

---

## 🎨 Color Themes

The application includes built-in 24-bit RGB color schemes configured at startup or via `config.h`:

- **Amber (Default)**: Warm amber focus phase (`#DC8C28`), emerald short break (`#46D282`), and cyan long break (`#50A0F0`).
- **Tron**: Futuristic neon cyan focus (`#00FFFF`), blue short break (`#0096FF`), and deep blue long break (`#0032FF`).
- **Stevan Mode**: Custom RGB theme scheme preset for personalized palette configurations.

---

## 🔄 Dependency Flow

```
pomodoro.cpp  ──────►  timer.h  ───►  config.h
      │                  ▲               ▲
      ├──────► renderer.h┼───────────────┤
      │            │     │               │
      ├──────► splash.h  │               │
      │            │     │               │
      └──────► term.h ───┴───► ansi.h ───┘
```
