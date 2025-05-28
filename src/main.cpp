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
    void restore() {}
    bool kbhit() { return _kbhit() != 0; }
    int getch() { return _getch(); }
#else
    void init() {}
    void restore() {}
    bool kbhit() { return _kbhit() != 0; }
    int getch() { return _getch(); }
#endif
}
int main() {
    return 0;
}