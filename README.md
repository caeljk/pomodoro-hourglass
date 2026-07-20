# Pomodoro Hourglass Focus Timer

A modern C++ TUI Pomodoro timer featuring a depleting ASCII hourglass and dynamic side-by-side digital clock display. Built using pure ANSI escape codes without external dependencies.

## Features
- Dynamic depleting sand visualization matching session progress
- Color-coded phases (Amber = Focus, Mint = Short Break, Sky Blue = Long Break)
- Large block-digit countdown clock
- Developer boot status splash sequence
- Live system wall-clock tracking

## Controls
- [space] - Pause / resume
- [s] - Skip current session
- [r] - Reset current timer
- [q] - Quit application

## Build Instructions

### Prerequisites
- A modern C++ compiler supporting C++17 (`g++`, `clang++`, or `MSVC`)
- CMake 3.16+ (optional, for project generation)

### Option 1: Quick compile (Direct G++)
Open your terminal in this directory and run:
```bash
# create build folder
mkdir build

# compile
g++ -std=c++17 -O2 -o build/pomodoro src/main.cpp
```

### Option 2: CMake build
Generate build scripts and compile using CMake:
```bash
# configure build structure
cmake -S . -B build

# compile the executable
cmake --build build --config Release
```

Once built, you can run the executable directly from the `build` directory:
```bash
./build/pomodoro
# On Windows command prompt/PowerShell:
# .\build\pomodoro.exe
```

### Stevan Mode

### this mode is an addition requested by stevan


ive made a new namespace that is for color themes: 
```
`constexpr Theme STEVAN_MODE = { {255, 0, 0}, {0, 255, 0}, {0, 0, 255} };`
```

this provides the user with the ability to instantiate the 24-bit RGB channels for the three elements.
but named with a twist, and i haven't added YAML abilities to pass variables by file.


