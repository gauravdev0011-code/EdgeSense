# EdgeSense — Real-Time Edge AI Sensor Fusion & Anomaly Detection

EdgeSense is a real-time edge-AI pipeline that coordinates **8 simulated sensor streams** through concurrent ingestion, buffering, feature extraction, and ML inference. It combines a multithreaded C++20 data path with PyTorch/ONNX Runtime inference, WebSocket telemetry, and a React dashboard.

## Resume-Aligned Summary

- Engineered a **multithreaded C++20 pipeline for 8 simulated sensor streams**, coordinating concurrent ingestion, buffering, feature extraction, and ML inference.
- Integrated **PyTorch and ONNX Runtime** for edge inference; benchmarked inference throughput and **p95 latency across 8-stream workloads**.
- Built real-time **WebSocket telemetry** and a React dashboard for sensor streams, inference latency, resource utilization, and anomaly scores.
- Evaluated **10K+ labeled sensor windows** using precision, recall, F1, false-positive rate, and false-negative rate.

## Architecture

```text
8 simulated sensor streams
        |
        v
Threaded ingestion -> thread-safe queue -> timestamp synchronization
        |
        v
Feature extraction -> scaler -> ONNX Runtime inference
        |
        +---------------------> WebSocket telemetry :9002
                                      |
                                      v
                                 React dashboard
```

## Repository Layout

```text
EdgeSense/
├── engine/
├── server/
├── frontend/
├── ml/
│   ├── data/sensor_data.csv
│   ├── train_model.py
│   ├── evaluate_model.py
│   ├── export_onnx.py
│   ├── benchmark_onnx.py
│   └── requirements.txt
├── third_party/
├── CMakeLists.txt
└── README.md
```

## ML Pipeline

The labeled dataset contains four sensor features and a binary anomaly label:

- temperature
- vibration
- current
- acceleration
- label

The evaluation workflow operates on the repository's **10K+ labeled sensor windows** and reports accuracy, precision, recall, F1, false-positive rate, false-negative rate, and the confusion matrix.

Training uses `StandardScaler`; the same preprocessing parameters are persisted for native C++ inference so training and deployed inference use consistent feature normalization.

### Train and evaluate

```bash
cd ml
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
python train_model.py
python evaluate_model.py
```

### Export and benchmark

```bash
python export_onnx.py
python benchmark_onnx.py
```

The ONNX benchmark runs an **8-stream workload** and reports p50/p95/p99 inference latency and estimated sensor-vector throughput. The resume's performance figures are environment-specific measurements and should be reproduced on the target machine before being quoted independently.

## Native Build

EdgeSense uses C++20, CMake, WebSocket++, standalone Asio, and ONNX Runtime.

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install -y build-essential cmake libasio-dev libwebsocketpp-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/edgesense
```

The C++ process starts the WebSocket server on port `9002` and broadcasts JSON telemetry containing sensor features, anomaly probability, anomaly status, and pipeline processing time.

## Frontend

```bash
cd frontend
npm install
npm run dev
```

The React dashboard connects to `ws://localhost:9002` and displays live sensor streams, anomaly status, ONNX inference output, pipeline processing time, and connection status.

## Engineering Details

### Concurrency

C++ threads process the simulated sensor streams concurrently. Readings enter a thread-safe queue before timestamp synchronization and feature extraction.

### Synchronization

Timestamp-based grouping keeps downstream feature extraction operating on coherent sensor windows rather than arbitrary arrival order.

### Inference

The PyTorch model is exported to ONNX. The C++ inference path applies the saved training scaler before passing the four-feature vector to ONNX Runtime.

### Telemetry

The C++ server maintains connected WebSocket clients and broadcasts JSON telemetry for each processing cycle. The React frontend consumes those messages through the browser WebSocket API.

## Validation and Benchmarking

| Metric | Resume-aligned workload |
|---|---:|
| Simulated sensor streams | **8** |
| Labeled sensor windows | **10K+** |
| ML evaluation metrics | **Precision, recall, F1, FPR, FNR** |
| Runtime metrics | **Throughput, p50, p95, p99 latency** |

## Project Status

The core sensor-processing, ML, inference, WebSocket, and dashboard path is implemented. Further work includes broader sensor models, stronger automated test coverage, and deployment hardening.

## Author

Gaurav Dev
