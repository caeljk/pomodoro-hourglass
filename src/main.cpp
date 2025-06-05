#include <iostream>
#include <string>
#ifdef _WIN32
#  define NOMINMAX
#  include <windows.h>
#  include <conio.h>
#else
#  include <termios.h>
#  include <unistd.h>
#endif

namespace term {
#ifdef _WIN32
    void init() {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        GetConsoleMode(hOut, &mode);
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    void restore() {
        tcsetattr(STDIN_FILENO, TCSANOW, &saved);
    }
    bool kbhit() { return _kbhit() != 0; }
    int getch() { return _getch(); }
#else
    static termios saved;
    void init() {
        tcgetattr(STDIN_FILENO, &saved);
        termios t = saved;
        t.c_lflag &= ~(ECHO | ICANON);
        t.c_cc[VMIN]  = 0;
        t.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
    }
    void restore() {
        tcsetattr(STDIN_FILENO, TCSANOW, &saved);
    }
    bool kbhit() { return _kbhit() != 0; }
    int getch() { return _getch(); }
#endif
}
int main() {
    return 0;
}