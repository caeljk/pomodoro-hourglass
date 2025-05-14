#include <iostream>
#include <string>
#ifdef _WIN32
#  define NOMINMAX
#  include <windows.h>
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
    bool kbhit() { return false; }
    int getch() { return 0; }
#else
    void init() {}
    void restore() {}
    bool kbhit() { return false; }
    int getch() { return 0; }
#endif
}
int main() {
    return 0;
}