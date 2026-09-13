cmake_minimum_required(VERSION 3.28)

project(EdgeSense LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_path(ASIO_INCLUDE_DIR asio.hpp REQUIRED)
find_path(WEBSOCKETPP_INCLUDE_DIR websocketpp/server.hpp REQUIRED)

set(ONNXRUNTIME_DIR
    ${CMAKE_SOURCE_DIR}/third_party/onnxruntime-linux-x64-1.29.0
)

add_executable(edgesense
    engine/src/main.cpp
    engine/src/Sensor.cpp
    engine/src/SensorSimulator.cpp
    engine/src/SensorQueue.cpp
    engine/src/SensorSynchronizer.cpp
    engine/src/FeatureExtractor.cpp
    engine/src/InferenceEngine.cpp
    server/src/WebSocketServer.cpp
)

# WebSocket++ 0.8.2 uses legacy constructor/destructor syntax that is
# rejected by modern compilers in C++20 mode. Keep the application C++20,
# but compile the isolated compatibility wrapper with C++17.
set_source_files_properties(
    server/src/WebSocketServer.cpp
    PROPERTIES
        COMPILE_OPTIONS "-std=c++17"
)

target_include_directories(edgesense PRIVATE
    engine/include
    server/include
    ${ASIO_INCLUDE_DIR}
    ${WEBSOCKETPP_INCLUDE_DIR}
    ${ONNXRUNTIME_DIR}/include
)

target_link_directories(edgesense PRIVATE
    ${ONNXRUNTIME_DIR}/lib
)

target_link_libraries(edgesense PRIVATE
    onnxruntime
    Threads::Threads
)

target_compile_definitions(edgesense PRIVATE
    ASIO_STANDALONE
)

target_compile_options(edgesense PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

set_target_properties(edgesense PROPERTIES
    BUILD_RPATH "${ONNXRUNTIME_DIR}/lib"
)
