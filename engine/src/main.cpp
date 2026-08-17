#include "FeatureExtractor.hpp"
#include "SensorQueue.hpp"
#include "SensorSimulator.hpp"
#include "SensorSynchronizer.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    using namespace edgesense;

    const auto pipelineStart = std::chrono::steady_clock::now();

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

    const auto pipelineEnd = std::chrono::steady_clock::now();

    const double pipelineTimeMs =
        std::chrono::duration<double, std::milli>(
            pipelineEnd - pipelineStart
        ).count();

    std::cout << std::fixed << std::setprecision(3);

    std::cout << "\n========================================\n";
    std::cout << "        EdgeSense Sensor Pipeline\n";
    std::cout << "========================================\n\n";

    std::cout << "Total readings       : "
              << readings.size()
              << '\n';

    std::cout << "Synchronized readings: "
              << window.readings.size()
              << '\n';

    std::cout << "\n--- Extracted Features ---\n";

    std::cout << "Temperature   : "
              << features.temperature
              << '\n';

    std::cout << "Vibration     : "
              << features.vibration
              << '\n';

    std::cout << "Current       : "
              << features.current
              << '\n';

    std::cout << "Acceleration  : "
              << features.acceleration
              << '\n';

    std::cout << "\n--- Pipeline Performance ---\n";

    std::cout << "Processing time: "
              << pipelineTimeMs
              << " ms\n";

    std::cout << "\nEdgeSense pipeline completed successfully.\n";

    return 0;
}