#pragma once

#include "SensorFeatures.hpp"
#include "SensorWindow.hpp"

namespace edgesense {

class FeatureExtractor {
public:
    SensorFeatures extract(const SensorWindow& window) const;
};

}