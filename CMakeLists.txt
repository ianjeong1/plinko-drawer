cmake_minimum_required(VERSION 3.10)
project(plinko-drawer)

set(CMAKE_CXX_STANDARD 17)
find_package(SFML 2.5 REQUIRED graphics window system)

add_executable(plinko-drawer
    src/main.cpp
    src/Game.cpp
    src/Ball.cpp
    src/Line.cpp
    src/Bucket.cpp
    src/Math.cpp
)

target_link_libraries(plinko-drawer sfml-graphics sfml-window sfml-system)