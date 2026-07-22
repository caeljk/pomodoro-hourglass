cmake_minimum_required(VERSION 3.16)
project(pomodoro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add modular source files
add_executable(pomodoro
    src/pomodoro.cpp
    src/timer.cpp
    src/renderer.cpp
    src/splash.cpp
)

# Include src directory for headers
target_include_directories(pomodoro PRIVATE src)