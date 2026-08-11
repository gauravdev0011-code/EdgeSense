#pragma once

#include "Sensor.hpp"

#include <random>
#include <string>

namespace edgesense {

class SensorSimulator {
public:
    SensorSimulator(const std::string& sensorId, SensorType type);

    SensorReading read();

private:
    std::string sensorId;
    SensorType type;
    std::uint64_t sequence = 0;

    std::default_random_engine generator;
    std::normal_distribution<double> distribution;
};

}