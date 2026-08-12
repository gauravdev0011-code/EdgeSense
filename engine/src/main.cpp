#include "SensorSimulator.hpp"

#include <iostream>
#include <thread>
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

    std::vector<std::thread> threads;

    for (auto& sensor : sensors) {
        threads.emplace_back([&sensor]() {
            SensorReading reading = sensor.read();

            std::cout << reading.sensorId
                      << " = "
                      << reading.value
                      << '\n';
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}