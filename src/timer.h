#ifndef POMODORO_TIMER_H
#define POMODORO_TIMER_H

#include "config.h"

/**
 * @file timer.h
 * @brief Pomodoro timer state machine tracking session phases, durations, and progress.
 */

/// Represents the active phase of the Pomodoro cycle.
enum class Phase { WORK, SHORT_BREAK, LONG_BREAK };

/// Core timer state machine structure.
struct Timer {
  int work_s = 25 * 60;   ///< Work session duration in seconds (25 mins)
  int short_s = 5 * 60;   ///< Short break duration in seconds (5 mins)
  int long_s = 15 * 60;   ///< Long break duration in seconds (15 mins)
  int cycle = 4;          ///< Work sessions required before triggering a long break

  Phase phase = Phase::WORK; ///< Current active phase
  int done = 0;              ///< Total completed work sessions
  int elapsed = 0;           ///< Seconds elapsed in current phase
  bool paused = false;       ///< Whether timer tick countdown is paused
  bool quit = false;         ///< Signal flag to terminate application loop

  /// Get total duration of the current phase in seconds.
  int total() const;

  /// Get remaining time in seconds for the current phase.
  int remaining() const;

  /// Get fractional progress [0.0 - 1.0] of current phase.
  double progress() const;

  /// Advance time by 1 second if unpaused.
  void tick();

  /// Transition to the next phase based on completed cycles.
  void advance();

  /// Reset current phase elapsed time and unpause.
  void reset();

  /// Force skip current phase and advance to next phase.
  void skip();

  /// Return string representation of current phase name.
  const char *name() const;
};

#endif // POMODORO_TIMER_H

