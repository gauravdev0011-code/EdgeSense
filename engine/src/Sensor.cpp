#include "Sensor.hpp"

namespace edgesense {

const char* sensorTypeToString(SensorType type) {
    switch (type) {
        case SensorType::Temperature:
            return "temperature";

        case SensorType::Vibration:
            return "vibration";

        case SensorType::Current:
            return "current";

        case SensorType::Acceleration:
            return "acceleration";
    }

    return "unknown";
}

}