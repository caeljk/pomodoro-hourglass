# Pomodoro Hourglass Focus Timer

A modern C++ TUI Pomodoro focus timer featuring a dynamic depleting ASCII hourglass visualization, big-digit clock display, live system wall-clock, custom color themes, and non-blocking terminal controls. Built using pure ANSI escape codes without third-party visual library dependencies.

---

## 📁 Project Layout

- **`src/modular-version/`**: **(Recommended)** Fully modular C++20 refactored implementation split into clean header and source files (`pomodoro.cpp`, `timer.h`/`timer.cpp`, `renderer.h`/`renderer.cpp`, `config.h`, `term.h`, `splash.h`/`splash.cpp`, `ansi.h`).
- **`src/main.cpp`**: Monolithic single-file C++ implementation.

---

## 🏗️ Modular Architecture (`src/modular-version`)

| Module File | Contents & Responsibilities |
|---|---|
| **`config.h`** | `Color` (RGB) & `Theme` structs, presets (`THEME_AMBER`, `THEME_TRON`, `STEVAN_MODE`), global `inline Theme active` |
| **`ansi.h`** | ANSI escape sequence helper functions (`mv`, `rgb`, screen clear, cursor toggles) |
| **`term.h`** | Cross-platform raw terminal abstraction (Win32 & POSIX support for `init`, `restore`, `kbhit`, `getch`, `size`, `beep`) |
| **`timer.h` / `timer.cpp`** | `Phase` enum (Work, Short Break, Long Break), `Timer` state machine logic, duration calculations, tick countdown, auto phase advance |
| **`renderer.h` / `renderer.cpp`** | Hourglass math (`sand_top`, `sand_bot`), 5-line big digit glyphs (`DG`), live wall clock, full UI layout composition |
| **`splash.h` / `splash.cpp`** | Startup splash screen animation, boot checklist, and interactive theme selector |
| **`pomodoro.cpp`** | Entry point `main()`, signal handling (`SIGINT`, `SIGTERM`), double-buffered output, 100ms render loop |

---

## ⚡ Quick Start & Compilation

To build and run the modular version:

```bash
cd src/modular-version

# Compile with C++20 and optimizations
g++ -std=c++20 -O3 -Wall \
    pomodoro.cpp timer.cpp renderer.cpp splash.cpp \
    -o pomodoro

# Run the executable
./pomodoro
```

---

## 🎮 Controls

| Key | Action |
|---|---|
| `<Space>` | Pause / resume countdown |
| `s` | Skip to next phase |
| `r` | Reset current timer |
| `q` | Quit application |

---

## 🎨 Theme Support & Customization

The timer supports multiple themes via `config.h`:
- **Amber (Default)**: Classic warm focus theme.
- **Tron**: Neon cyan / deep blue theme.
- **Stevan Mode**: Custom palette scheme (`constexpr Theme STEVAN_MODE`).
