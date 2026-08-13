#include "SensorSynchronizer.hpp"

namespace edgesense {

SensorSynchronizer::SensorSynchronizer(
    std::chrono::milliseconds windowSize
)
    : windowSize(windowSize) {
}

SensorWindow SensorSynchronizer::createWindow(
    const std::vector<SensorReading>& readings
) {
    SensorWindow window;

    if (readings.empty()) {
        return window;
    }

    auto startTime = readings.front().timestamp;

    for (const auto& reading : readings) {
        auto difference = reading.timestamp - startTime;

        if (difference <= windowSize) {
            window.readings.push_back(reading);
        }
    }

    return window;
}

}