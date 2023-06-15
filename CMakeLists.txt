cmake_minimum_required(VERSION 3.16)
project(asteroides)

set(CMAKE_CXX_STANDARD 17)

add_executable(asteroides engine.c game.c MenuWindow.h)
include_directories(c:\\mingw64\\include)
link_directories(c:\\mingw64\\bin)
target_link_libraries(asteroides mingw32 SDL2main SDL2 SDL2_image SDL2_ttf SDL2_mixer )