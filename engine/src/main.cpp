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
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

namespace {

std::atomic<bool> running{true};

void handleSignal(int) {
    running = false;
}

} // namespace

int main() {
    using namespace edgesense;

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    // ------------------------------------------------------------
    // Start WebSocket telemetry server
    // ------------------------------------------------------------

    WebSocketServer webSocketServer(9002);
    webSocketServer.start();

    // Give the WebSocket server time to initialize.
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100)
    );

    // ------------------------------------------------------------
    // Create sensor simulators
    // ------------------------------------------------------------

    std::vector<SensorSimulator> sensors = {
        SensorSimulator(
            "temperature-01",
            SensorType::Temperature
        ),
        SensorSimulator(
            "temperature-02",
            SensorType::Temperature
        ),
        SensorSimulator(
            "vibration-01",
            SensorType::Vibration
        ),
        SensorSimulator(
            "vibration-02",
            SensorType::Vibration
        ),
        SensorSimulator(
            "current-01",
            SensorType::Current
        ),
        SensorSimulator(
            "current-02",
            SensorType::Current
        ),
        SensorSimulator(
            "acceleration-01",
            SensorType::Acceleration
        ),
        SensorSimulator(
            "acceleration-02",
            SensorType::Acceleration
        )
    };

    // ------------------------------------------------------------
    // Create pipeline components once
    // ------------------------------------------------------------

    SensorSynchronizer synchronizer(
        std::chrono::milliseconds(100)
    );

    FeatureExtractor extractor;

    InferenceEngine inference(
        "ml/anomaly_model.onnx"
    );

    std::cout
        << "\n========================================\n"
        << "        EdgeSense Sensor Pipeline\n"
        << "========================================\n\n";

    std::cout
        << "[EdgeSense] Live telemetry mode enabled.\n"
        << "[EdgeSense] WebSocket endpoint: ws://localhost:9002\n"
        << "[EdgeSense] Press Ctrl+C to stop.\n\n";

    std::uint64_t cycle = 0;

    // ------------------------------------------------------------
    // Continuous telemetry loop
    // ------------------------------------------------------------

    while (running) {
        ++cycle;

        const auto pipelineStart =
            std::chrono::steady_clock::now();

        // --------------------------------------------------------
        // Collect sensor readings concurrently
        // --------------------------------------------------------

        SensorQueue queue;
        std::vector<std::thread> threads;

        threads.reserve(sensors.size());

        for (auto& sensor : sensors) {
            threads.emplace_back(
                [&sensor, &queue]() {
                    SensorReading reading = sensor.read();
                    queue.push(reading);
                }
            );
        }

        for (auto& thread : threads) {
            thread.join();
        }

        // --------------------------------------------------------
        // Drain queue
        // --------------------------------------------------------

        std::vector<SensorReading> readings;
        SensorReading reading;

        while (queue.pop(reading)) {
            readings.push_back(reading);
        }

        // --------------------------------------------------------
        // Synchronize readings
        // --------------------------------------------------------

        SensorWindow window =
            synchronizer.createWindow(readings);

        // --------------------------------------------------------
        // Extract features
        // --------------------------------------------------------

        SensorFeatures features =
            extractor.extract(window);

        // --------------------------------------------------------
        // ML inference
        // --------------------------------------------------------

        const double anomalyLogit =
            inference.predict(features);

        const double anomalyProbability =
            1.0 / (1.0 + std::exp(-anomalyLogit));

        const bool anomalyDetected =
            anomalyProbability >= 0.5;

        // --------------------------------------------------------
        // Pipeline timing
        // --------------------------------------------------------

        const auto pipelineEnd =
            std::chrono::steady_clock::now();

        const double pipelineTimeMs =
            std::chrono::duration<double, std::milli>(
                pipelineEnd - pipelineStart
            ).count();

        // --------------------------------------------------------
        // Console telemetry
        // --------------------------------------------------------

        std::cout
            << std::fixed
            << std::setprecision(3);

        std::cout
            << "\n----------------------------------------\n"
            << "Telemetry Cycle: "
            << cycle
            << '\n'
            << "----------------------------------------\n";

        std::cout
            << "Total readings       : "
            << readings.size()
            << '\n';

        std::cout
            << "Synchronized readings: "
            << window.readings.size()
            << '\n';

        std::cout
            << "\n--- Extracted Features ---\n";

        std::cout
            << "Temperature   : "
            << features.temperature
            << '\n';

        std::cout
            << "Vibration     : "
            << features.vibration
            << '\n';

        std::cout
            << "Current       : "
            << features.current
            << '\n';

        std::cout
            << "Acceleration  : "
            << features.acceleration
            << '\n';

        std::cout
            << "\n--- ML Inference ---\n";

        std::cout
            << "Anomaly logit       : "
            << anomalyLogit
            << '\n';

        std::cout
            << "Anomaly probability : "
            << anomalyProbability
            << '\n';

        std::cout
            << "Anomaly detected    : "
            << (anomalyDetected ? "YES" : "NO")
            << '\n';

        std::cout
            << "\n--- Pipeline Performance ---\n";

        std::cout
            << "Processing time: "
            << pipelineTimeMs
            << " ms\n";

        // --------------------------------------------------------
        // Build telemetry JSON
        // --------------------------------------------------------

        std::ostringstream telemetry;

        telemetry
            << std::fixed
            << std::setprecision(3)
            << "{"
            << "\"cycle\":"
            << cycle
            << ",\"temperature\":"
            << features.temperature
            << ",\"vibration\":"
            << features.vibration
            << ",\"current\":"
            << features.current
            << ",\"acceleration\":"
            << features.acceleration
            << ",\"anomaly_logit\":"
            << anomalyLogit
            << ",\"anomaly_probability\":"
            << anomalyProbability
            << ",\"anomaly_detected\":"
            << (anomalyDetected ? "true" : "false")
            << ",\"processing_time_ms\":"
            << pipelineTimeMs
            << "}";

        // --------------------------------------------------------
        // Broadcast telemetry to WebSocket clients
        // --------------------------------------------------------

        webSocketServer.broadcast(
            telemetry.str()
        );

        std::cout
            << "[EdgeSense] Telemetry broadcast complete.\n";

        // --------------------------------------------------------
        // Run approximately once per second
        // --------------------------------------------------------

        constexpr auto telemetryInterval =
            std::chrono::seconds(1);

        std::this_thread::sleep_for(
            telemetryInterval
        );
    }

    // ------------------------------------------------------------
    // Graceful shutdown
    // ------------------------------------------------------------

    std::cout
        << "\n[EdgeSense] Shutdown requested.\n";

    webSocketServer.stop();

    std::cout
        << "[EdgeSense] Telemetry server stopped.\n"
        << "[EdgeSense] Pipeline shutdown complete.\n";

    return 0;
}