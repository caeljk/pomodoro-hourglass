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
namespace term {
    void beep() {
#ifdef _WIN32
        Beep(880, 200);
#else
        std::cout << '\a' << std::flush;
#endif
    }
}
enum class Phase { WORK, SHORT_BREAK, LONG_BREAK };
struct Timer {
    int work_s  = 25 * 60;
    int short_s =  5 * 60;
    int long_s  = 15 * 60;
    Phase phase   = Phase::WORK;
    int   done    = 0;
    int   elapsed = 0;
    bool  paused  = false;
    bool  quit    = false;
    int total() const {
        if (phase == Phase::SHORT_BREAK) return short_s;
        if (phase == Phase::LONG_BREAK) return long_s;
        return work_s;
    }
    int remaining() const { return total() - elapsed; }
    double progress() const { return (double)elapsed / total(); }
    void advance() {
        if (phase == Phase::WORK) {
            ++done;
            phase = (done % 4 == 0) ? Phase::LONG_BREAK : Phase::SHORT_BREAK;
        } else {
            phase = Phase::WORK;
        }
        elapsed = 0;
    }
    void tick() {
        if (paused || quit) return;
        if (++elapsed >= total()) {
            advance();
        }
    }
};
namespace hg {
    const int HALF = 8;
    const int IMAX = 16;
    inline int iw(int r) { return IMAX - 2 * (r - 1); }
}