#include "timer.h"
#include "term.h"
#include <algorithm>

/**
 * @file timer.cpp
 * @brief Implementation of Timer methods and state transition logic.
 */

// Returns total duration for the currently active phase in seconds
int Timer::total() const {
  if (phase == Phase::SHORT_BREAK)
    return short_s;
  if (phase == Phase::LONG_BREAK)
    return long_s;
  return work_s;
}

// Calculates remaining seconds until phase completion
int Timer::remaining() const { 
  return total() - elapsed; 
}

// Calculates ratio of completed phase time [0.0 - 1.0] for sand rendering
double Timer::progress() const {
  return std::min(1.0, (double)elapsed / total());
}

// Advances countdown by 1 second if active; triggers sound and phase transition on zero
void Timer::tick() {
  // Do nothing if paused or application quit signal set
  if (paused || quit)
    return;

  // Increment elapsed seconds and check if current phase completed
  if (++elapsed >= total()) {
    term::beep(); // Play completion sound chime
    advance();    // Transition to next phase in Pomodoro cycle
  }
}

// State machine transition logic between Work, Short Break, and Long Break
void Timer::advance() {
  if (phase == Phase::WORK) {
    ++done; // Record completed focus session
    // After 'cycle' (4) work sessions, switch to long break; otherwise short break
    phase = (done % cycle == 0) ? Phase::LONG_BREAK : Phase::SHORT_BREAK;
  } else {
    // Break finished, return to focus work session
    phase = Phase::WORK;
  }
  elapsed = 0; // Reset phase elapsed counter
}

// Reset current session timer back to start and unpause
void Timer::reset() {
  elapsed = 0;
  paused = false;
}

// Manually skip current phase and advance to next phase
void Timer::skip() {
  if (phase == Phase::WORK)
    ++done; // Count skipped work session as completed
  advance();
}

// Human-readable string representation of current active phase
const char *Timer::name() const {
  if (phase == Phase::SHORT_BREAK)
    return "short break";
  if (phase == Phase::LONG_BREAK)
    return "long break";
  return "focus";
}


