cmake_minimum_required(VERSION 3.28)

project(EdgeSense LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
)

target_include_directories(edgesense PRIVATE
    engine/include
    ${ONNXRUNTIME_DIR}/include
)

target_link_directories(edgesense PRIVATE
    ${ONNXRUNTIME_DIR}/lib
)

target_link_libraries(edgesense PRIVATE
    onnxruntime
)

target_compile_options(edgesense PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

set_target_properties(edgesense PROPERTIES
    BUILD_RPATH "${ONNXRUNTIME_DIR}/lib"
)