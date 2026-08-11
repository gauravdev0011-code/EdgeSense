cmake_minimum_required(VERSION 3.28)

project(EdgeSense LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(edgesense
    engine/src/main.cpp
    engine/src/Sensor.cpp
    engine/src/SensorSimulator.cpp
)

target_include_directories(edgesense PRIVATE engine/include)

target_compile_options(edgesense PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)