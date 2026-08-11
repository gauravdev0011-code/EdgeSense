#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace edgesense {

enum class SensorType {
    Temperature,
    Vibration,
    Current,
    Acceleration
};

struct SensorReading {
    std::uint64_t sequence;
    std::chrono::steady_clock::time_point timestamp;
    std::string sensorId;
    SensorType type;
    double value;
};

const char* sensorTypeToString(SensorType type);

}