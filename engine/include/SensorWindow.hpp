#pragma once

#include "Sensor.hpp"

#include <vector>

namespace edgesense {

struct SensorWindow {
    std::vector<SensorReading> readings;
};

}