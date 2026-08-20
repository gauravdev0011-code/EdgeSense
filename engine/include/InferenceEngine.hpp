#pragma once

#include "SensorFeatures.hpp"

#include <onnxruntime_cxx_api.h>

#include <string>

namespace edgesense {

class InferenceEngine {
public:
    explicit InferenceEngine(const std::string& modelPath);

    float predict(const SensorFeatures& features);

private:
    Ort::Env env;
    Ort::SessionOptions sessionOptions;
    Ort::Session session{nullptr};

    std::string inputName;
    std::string outputName;
};

} // namespace edgesense
