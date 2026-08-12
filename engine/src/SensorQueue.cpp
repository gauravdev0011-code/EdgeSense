#include "SensorQueue.hpp"

namespace edgesense {

void SensorQueue::push(const SensorReading& reading) {
    std::lock_guard<std::mutex> lock(mutex);
    readings.push(reading);
}

bool SensorQueue::pop(SensorReading& reading) {
    std::lock_guard<std::mutex> lock(mutex);

    if (readings.empty()) {
        return false;
    }

    reading = readings.front();
    readings.pop();

    return true;
}

}