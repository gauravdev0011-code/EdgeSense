#include "SensorSimulator.hpp"

#include <chrono>

namespace edgesense {

SensorSimulator::SensorSimulator(
    const std::string& sensorId,
    SensorType type)
    : sensorId(sensorId),
      type(type),
      distribution(0.0, 1.0) {
}

SensorReading SensorSimulator::read() {
    SensorReading reading{
        sequence++,
        std::chrono::steady_clock::now(),
        sensorId,
        type,
        distribution(generator)
    };

    return reading;
}

}