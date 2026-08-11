#include "SensorSimulator.hpp"

#include <iostream>
#include <vector>

int main() {
    using namespace edgesense;

    std::vector<SensorSimulator> sensors = {
        SensorSimulator("temperature-01", SensorType::Temperature),
        SensorSimulator("temperature-02", SensorType::Temperature),
        SensorSimulator("vibration-01", SensorType::Vibration),
        SensorSimulator("vibration-02", SensorType::Vibration),
        SensorSimulator("current-01", SensorType::Current),
        SensorSimulator("current-02", SensorType::Current),
        SensorSimulator("acceleration-01", SensorType::Acceleration),
        SensorSimulator("acceleration-02", SensorType::Acceleration)
    };

    for (auto& sensor : sensors) {
        SensorReading reading = sensor.read();

        std::cout << reading.sensorId
                  << " = "
                  << reading.value
                  << '\n';
    }

    return 0;
}