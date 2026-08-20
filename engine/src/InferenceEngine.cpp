#include "InferenceEngine.hpp"

#include <array>
#include <iostream>
#include <stdexcept>

namespace edgesense {

InferenceEngine::InferenceEngine(const std::string& modelPath)
    : env(ORT_LOGGING_LEVEL_WARNING, "EdgeSense"),
      sessionOptions(),
      session(nullptr) {

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

    std::array<float, 4> inputValues = {
        static_cast<float>(features.temperature),
        static_cast<float>(features.vibration),
        static_cast<float>(features.current),
        static_cast<float>(features.acceleration)
    };

    std::array<int64_t, 2> inputShape = {1, 4};

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

    const char* inputNames[] = {
        inputName.c_str()
    };

    const char* outputNames[] = {
        outputName.c_str()
    };

    auto outputTensors = session.Run(
        Ort::RunOptions{nullptr},
        inputNames,
        &inputTensor,
        1,
        outputNames,
        1
    );

    if (outputTensors.empty() ||
        !outputTensors[0].IsTensor()) {
        throw std::runtime_error(
            "ONNX inference returned an invalid output."
        );
    }

    const float* outputData =
        outputTensors[0].GetTensorData<float>();

    return outputData[0];
}

} // namespace edgesense
