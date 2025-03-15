#include <iostream>
#ifdef _WIN32
#  define NOMINMAX
#  include <windows.h>
#endif
namespace ansi {
    constexpr const char* RST  = "\033[0m";
    constexpr const char* CLR  = "\033[2J\033[H";
}
int main() {
    std::cout << ansi::CLR << "Started\n";
    return 0;
}