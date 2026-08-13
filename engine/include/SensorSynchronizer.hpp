#pragma once

#include "SensorWindow.hpp"

#include <chrono>
#include <vector>

namespace edgesense {

class SensorSynchronizer {
public:
    explicit SensorSynchronizer(
        std::chrono::milliseconds windowSize
    );

    SensorWindow createWindow(
        const std::vector<SensorReading>& readings
    );

private:
    std::chrono::milliseconds windowSize;
};

}