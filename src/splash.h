#ifndef POMODORO_SPLASH_H
#define POMODORO_SPLASH_H

#include "term.h"
#include <iosfwd>

/**
 * @file splash.h
 * @brief Boot splash screen display and theme selection UI.
 */

/// Render interactive startup splash screen and process user theme selection.
void splash(int rows, int cols);

#endif // POMODORO_SPLASH_H

