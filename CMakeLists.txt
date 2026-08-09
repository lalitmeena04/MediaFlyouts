cmake_minimum_required(VERSION 3.21)

project(MediaFlyouts VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick Test)
qt_standard_project_setup(REQUIRES 6.5)

enable_testing()

add_subdirectory(src)
add_subdirectory(app)
add_subdirectory(tests)
