#pragma once

#include "Sensor.hpp"

#include <mutex>
#include <queue>

namespace edgesense {

class SensorQueue {
public:
    void push(const SensorReading& reading);
    bool pop(SensorReading& reading);

private:
    std::queue<SensorReading> readings;
    std::mutex mutex;
};

}