Here's a clean modular split for your Pomodoro timer:

## Proposed Structure

```
pomodoro/
├── pomodoro.cpp    # main(), event loop, input handling
├── timer.h         # Phase enum, Timer struct, state machine
├── timer.cpp       # Timer methods (tick, advance, reset, skip)
├── renderer.h      # draw_clock, draw_hourglass, render, wall_clock
├── renderer.cpp    # Big digit glyphs, hourglass math, render impl
├── config.h        # Color, Theme structs, theme constants
└── term.h          # Terminal abstraction (init, restore, kbhit, getch, beep, size)
```

## Responsibilities

| File | Contents |
|---|---|
| `config.h` | `Color`, `Theme`, `THEME_AMBER`, `THEME_TRON`, `STEVAN_MODE`, `GLASS`, `GRAIN` |
| `term.h` | `term::init()`, `term::restore()`, `term::kbhit()`, `term::getch()`, `term::beep()`, `term::size()` |
| `timer.h` | `Phase` enum, `Timer` struct, `total()`, `remaining()`, `progress()` |
| `timer.cpp` | `tick()`, `advance()`, `reset()`, `skip()`, `name()` |
| `renderer.h` | `hg::draw()`, `draw_clock()`, `wall_clock()`, `render()` |
| `renderer.cpp` | Hourglass math (`sand_top`, `sand_bot`, `iw`), digit glyphs, render impl |
| `pomodoro.cpp` | `splash()`, `main()` event loop, signal handling |

## Key Dependencies

```
pomodoro.cpp  →  timer.h, renderer.h, config.h, term.h
timer.cpp     →  timer.h, config.h
renderer.cpp  →  renderer.h, timer.h, config.h, term.h (for ansi)
term.cpp      →  term.h (platform-specific headers)
```

## Quick Refactor Steps

1. **Extract `config.h`** — move the `config` namespace to its own file
2. **Extract `term.h`** — move terminal platform layer
3. **Split `timer.h` / `timer.cpp`** — struct declaration in header, methods in cpp
4. **Split `renderer.h` / `renderer.cpp`** — all rendering functions
5. **Move `splash()`** to `pomodoro.cpp` — it's UI/boot logic
6. **`main()`** stays in `pomodoro.cpp` — orchestrates init → splash → loop → cleanup

Want me to generate the actual split files?
