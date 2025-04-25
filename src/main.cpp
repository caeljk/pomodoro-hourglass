#include <iostream>
#include <string>
#ifdef _WIN32
#  define NOMINMAX
#  include <windows.h>
#endif
namespace ansi {
    constexpr const char* EL   = "\033[K";
    inline std::string rgb(int r, int g, int b) {
        return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + ";m";
    }
    inline std::string mv(int r, int c) {
        return "\033[" + std::to_string(r) + ";" + std::to_string(c) + "H";
    }
    constexpr const char* RST  = "\033[0m";
    constexpr const char* CLR  = "\033[2J\033[H";
    constexpr const char* HIDE = "\033[?25l";
    constexpr const char* SHOW = "\033[?25h";
}
int main() {
    std::cout << ansi::CLR << ansi::mv(1, 1) << "Ready" << ansi::RST << std::endl;
    return 0;
}