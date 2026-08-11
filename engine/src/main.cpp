#include "Sensor.hpp"

#include <iostream>

int main() {
    using namespace edgesense;

    SensorReading reading{
        1,
        std::chrono::steady_clock::now(),
        "temperature-01",
        SensorType::Temperature,
        72.4
    };

    std::cout << "EdgeSense engine initialized\n";
    std::cout << "Sensor: " << reading.sensorId << '\n';
    std::cout << "Type: " << sensorTypeToString(reading.type) << '\n';
    std::cout << "Value: " << reading.value << '\n';

    return 0;
}