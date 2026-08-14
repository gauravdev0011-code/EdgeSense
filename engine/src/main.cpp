#include "FeatureExtractor.hpp"
#include "SensorQueue.hpp"
#include "SensorSimulator.hpp"
#include "SensorSynchronizer.hpp"

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

    SensorQueue queue;
    std::vector<std::thread> threads;

    for (auto& sensor : sensors) {
        threads.emplace_back([&sensor, &queue]() {
            SensorReading reading = sensor.read();
            queue.push(reading);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::vector<SensorReading> readings;
    SensorReading reading;

    while (queue.pop(reading)) {
        readings.push_back(reading);
    }

    SensorSynchronizer synchronizer(
        std::chrono::milliseconds(100)
    );

    SensorWindow window = synchronizer.createWindow(readings);

    FeatureExtractor extractor;
    SensorFeatures features = extractor.extract(window);

    std::cout << "Total readings: "
              << readings.size()
              << '\n';

    std::cout << "Synchronized readings: "
              << window.readings.size()
              << '\n';

    std::cout << "Features:\n";

    std::cout << "temperature = "
              << features.temperature
              << '\n';

    std::cout << "vibration = "
              << features.vibration
              << '\n';

    std::cout << "current = "
              << features.current
              << '\n';

    std::cout << "acceleration = "
              << features.acceleration
              << '\n';

    return 0;
}