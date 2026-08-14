#include "FeatureExtractor.hpp"

namespace edgesense {

SensorFeatures FeatureExtractor::extract(const SensorWindow& window) const {
    SensorFeatures features{
        0.0,
        0.0,
        0.0,
        0.0
    };

    int temperatureCount = 0;
    int vibrationCount = 0;
    int currentCount = 0;
    int accelerationCount = 0;

    for (const auto& reading : window.readings) {
        switch (reading.type) {
            case SensorType::Temperature:
                features.temperature += reading.value;
                ++temperatureCount;
                break;

            case SensorType::Vibration:
                features.vibration += reading.value;
                ++vibrationCount;
                break;

            case SensorType::Current:
                features.current += reading.value;
                ++currentCount;
                break;

            case SensorType::Acceleration:
                features.acceleration += reading.value;
                ++accelerationCount;
                break;
        }
    }

    if (temperatureCount > 0) {
        features.temperature /= temperatureCount;
    }

    if (vibrationCount > 0) {
        features.vibration /= vibrationCount;
    }

    if (currentCount > 0) {
        features.current /= currentCount;
    }

    if (accelerationCount > 0) {
        features.acceleration /= accelerationCount;
    }

    return features;
}

}