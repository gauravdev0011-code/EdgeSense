#include "SensorSimulator.hpp"

#include <iostream>

int main() {
    using namespace edgesense;

    SensorSimulator sensor(
        "temperature-01",
        SensorType::Temperature
    );

    for (int i = 0; i < 5; ++i) {
        SensorReading reading = sensor.read();

        std::cout << reading.sensorId
                  << " = "
                  << reading.value
                  << '\n';
    }

    return 0;
}