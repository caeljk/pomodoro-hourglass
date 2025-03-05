#include <iostream>
#ifdef _WIN32
#  define NOMINMAX
#  include <windows.h>
#endif
int main() {
    std::cout << "initialized
";
    return 0;
}