#pragma once

#include "SensorFeatures.hpp"

#include <onnxruntime_cxx_api.h>

#include <array>
#include <string>

namespace edgesense {

class InferenceEngine {
public:
    InferenceEngine(
        const std::string& modelPath,
        const std::string& scalerPath
    );

    float predict(const SensorFeatures& features);

private:
    Ort::Env env;
    Ort::SessionOptions sessionOptions;
    Ort::Session session{nullptr};

    std::string inputName;
    std::string outputName;

    std::array<float, 4> mean{};
    std::array<float, 4> scale{};
};

} // namespace edgesense
