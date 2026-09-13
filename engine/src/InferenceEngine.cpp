#include "InferenceEngine.hpp"

#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace edgesense {

InferenceEngine::InferenceEngine(
    const std::string& modelPath,
    const std::string& scalerPath
)
    : env(ORT_LOGGING_LEVEL_WARNING, "EdgeSense"),
      sessionOptions(),
      session(nullptr) {

    std::ifstream scalerFile(scalerPath);
    if (!scalerFile) {
        throw std::runtime_error(
            "Unable to open scaler file: " + scalerPath
        );
    }

    for (std::size_t i = 0; i < mean.size(); ++i) {
        if (!(scalerFile >> mean[i])) {
            throw std::runtime_error(
                "Invalid scaler mean values in: " + scalerPath
            );
        }
    }

    for (std::size_t i = 0; i < scale.size(); ++i) {
        if (!(scalerFile >> scale[i]) || scale[i] == 0.0F) {
            throw std::runtime_error(
                "Invalid scaler scale values in: " + scalerPath
            );
        }
    }

    sessionOptions.SetIntraOpNumThreads(1);
    sessionOptions.SetInterOpNumThreads(1);
    sessionOptions.SetGraphOptimizationLevel(
        GraphOptimizationLevel::ORT_ENABLE_ALL
    );

    session = Ort::Session(
        env,
        modelPath.c_str(),
        sessionOptions
    );

    Ort::AllocatorWithDefaultOptions allocator;

    auto inputNameAllocated =
        session.GetInputNameAllocated(0, allocator);
    auto outputNameAllocated =
        session.GetOutputNameAllocated(0, allocator);

    inputName = inputNameAllocated.get();
    outputName = outputNameAllocated.get();

    std::cout << "ONNX model loaded successfully.\n";
    std::cout << "Input : " << inputName << '\n';
    std::cout << "Output: " << outputName << '\n';
}

float InferenceEngine::predict(const SensorFeatures& features) {
    const std::array<float, 4> rawValues = {
        static_cast<float>(features.temperature),
        static_cast<float>(features.vibration),
        static_cast<float>(features.current),
        static_cast<float>(features.acceleration)
    };

    std::array<float, 4> inputValues{};
    for (std::size_t i = 0; i < inputValues.size(); ++i) {
        inputValues[i] = (rawValues[i] - mean[i]) / scale[i];
    }

    const std::array<int64_t, 2> inputShape = {1, 4};

    Ort::MemoryInfo memoryInfo =
        Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator,
            OrtMemTypeDefault
        );

    Ort::Value inputTensor =
        Ort::Value::CreateTensor<float>(
            memoryInfo,
            inputValues.data(),
            inputValues.size(),
            inputShape.data(),
            inputShape.size()
        );

    const char* inputNames[] = {inputName.c_str()};
    const char* outputNames[] = {outputName.c_str()};

    auto outputTensors = session.Run(
        Ort::RunOptions{nullptr},
        inputNames,
        &inputTensor,
        1,
        outputNames,
        1
    );

    if (outputTensors.empty() || !outputTensors[0].IsTensor()) {
        throw std::runtime_error(
            "ONNX inference returned an invalid output."
        );
    }

    const float* outputData =
        outputTensors[0].GetTensorData<float>();

    return outputData[0];
}

} // namespace edgesense
