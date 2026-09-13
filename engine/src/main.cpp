#include "FeatureExtractor.hpp"
#include "InferenceEngine.hpp"
#include "SensorQueue.hpp"
#include "SensorSimulator.hpp"
#include "SensorSynchronizer.hpp"
#include "WebSocketServer.hpp"

#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <thread>
#include <vector>

#include <sys/resource.h>

namespace {

std::atomic<bool> running{true};

void handleSignal(int) {
    running = false;
}

struct ProcessResources {
    double cpuSeconds{0.0};
    double memoryMb{0.0};
};

ProcessResources readProcessResources() {
    rusage usage{};
    getrusage(RUSAGE_SELF, &usage);

    const double cpuSeconds =
        static_cast<double>(usage.ru_utime.tv_sec) +
        static_cast<double>(usage.ru_utime.tv_usec) / 1'000'000.0 +
        static_cast<double>(usage.ru_stime.tv_sec) +
        static_cast<double>(usage.ru_stime.tv_usec) / 1'000'000.0;

    long memoryKb = 0;
    std::ifstream status("/proc/self/status");
    std::string key;
    while (status >> key) {
        if (key == "VmRSS:") {
            status >> memoryKb;
            break;
        }
        status.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return {cpuSeconds, static_cast<double>(memoryKb) / 1024.0};
}

} // namespace

int main() {
    using namespace edgesense;

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    WebSocketServer webSocketServer(9002);
    webSocketServer.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::vector<SensorSimulator> sensors = {
        SensorSimulator("temperature-01", SensorType::Temperature),
        SensorSimulator("temperature-02", SensorType::Temperature),
        SensorSimulator("vibration-01", SensorType::Vibration),
        SensorSimulator("vibration-02", SensorType::Vibration),
        SensorSimulator("current-01", SensorType::Current),
        SensorSimulator("current-02", SensorType::Current),
        SensorSimulator("acceleration-01", SensorType::Acceleration),
        SensorSimulator("acceleration-02", SensorType::Acceleration)
    };

    SensorSynchronizer synchronizer(std::chrono::milliseconds(100));
    FeatureExtractor extractor;

    InferenceEngine inference(
        "ml/anomaly_model.onnx",
        "ml/scaler.txt"
    );

    std::cout
        << "\n========================================\n"
        << "        EdgeSense Sensor Pipeline\n"
        << "========================================\n\n"
        << "[EdgeSense] Live telemetry mode enabled.\n"
        << "[EdgeSense] WebSocket endpoint: ws://localhost:9002\n"
        << "[EdgeSense] Press Ctrl+C to stop.\n\n";

    std::uint64_t cycle = 0;
    ProcessResources previousResources = readProcessResources();
    auto previousResourceTime = std::chrono::steady_clock::now();

    while (running) {
        ++cycle;
        const auto pipelineStart = std::chrono::steady_clock::now();

        SensorQueue queue;
        std::vector<std::thread> threads;
        threads.reserve(sensors.size());

        for (auto& sensor : sensors) {
            threads.emplace_back([&sensor, &queue]() {
                queue.push(sensor.read());
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        std::vector<SensorReading> readings;
        SensorReading reading;
        while (queue.pop(reading)) {
            readings.push_back(reading);
        }

        SensorWindow window = synchronizer.createWindow(readings);
        SensorFeatures features = extractor.extract(window);

        const double anomalyLogit = inference.predict(features);
        const double anomalyProbability =
            1.0 / (1.0 + std::exp(-anomalyLogit));
        const bool anomalyDetected = anomalyProbability >= 0.5;

        const auto pipelineEnd = std::chrono::steady_clock::now();
        const double pipelineTimeMs =
            std::chrono::duration<double, std::milli>(
                pipelineEnd - pipelineStart
            ).count();

        const ProcessResources currentResources = readProcessResources();
        const auto resourceTime = std::chrono::steady_clock::now();
        const double wallSeconds =
            std::chrono::duration<double>(resourceTime - previousResourceTime).count();
        const double cpuUtilizationPercent = wallSeconds > 0.0
            ? ((currentResources.cpuSeconds - previousResources.cpuSeconds) /
               wallSeconds) * 100.0
            : 0.0;
        previousResources = currentResources;
        previousResourceTime = resourceTime;

        std::cout << std::fixed << std::setprecision(3)
                  << "\nTelemetry Cycle: " << cycle << '\n'
                  << "Total readings       : " << readings.size() << '\n'
                  << "Synchronized readings: " << window.readings.size() << '\n'
                  << "Temperature          : " << features.temperature << '\n'
                  << "Vibration            : " << features.vibration << '\n'
                  << "Current              : " << features.current << '\n'
                  << "Acceleration         : " << features.acceleration << '\n'
                  << "Anomaly logit        : " << anomalyLogit << '\n'
                  << "Anomaly probability  : " << anomalyProbability << '\n'
                  << "Anomaly detected     : " << (anomalyDetected ? "YES" : "NO") << '\n'
                  << "Processing time      : " << pipelineTimeMs << " ms\n"
                  << "CPU utilization      : " << cpuUtilizationPercent << " %\n"
                  << "Memory RSS           : " << currentResources.memoryMb << " MB\n";

        std::ostringstream telemetry;
        telemetry << std::fixed << std::setprecision(3)
                  << "{"
                  << "\"cycle\":" << cycle
                  << ",\"temperature\":" << features.temperature
                  << ",\"vibration\":" << features.vibration
                  << ",\"current\":" << features.current
                  << ",\"acceleration\":" << features.acceleration
                  << ",\"anomaly_logit\":" << anomalyLogit
                  << ",\"anomaly_probability\":" << anomalyProbability
                  << ",\"anomaly_detected\":" << (anomalyDetected ? "true" : "false")
                  << ",\"processing_time_ms\":" << pipelineTimeMs
                  << ",\"cpu_utilization_percent\":" << cpuUtilizationPercent
                  << ",\"memory_mb\":" << currentResources.memoryMb
                  << "}";

        webSocketServer.broadcast(telemetry.str());
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    webSocketServer.stop();
    return 0;
}
