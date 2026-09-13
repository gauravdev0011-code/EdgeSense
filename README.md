# EdgeSense — Real-Time Edge AI Sensor Fusion & Anomaly Detection

EdgeSense is a real-time edge-AI pipeline that coordinates **8 simulated sensor streams** through concurrent ingestion, buffering, feature extraction, and ML inference. It combines a multithreaded C++20 data path with PyTorch/ONNX Runtime inference, WebSocket telemetry, Linux process metrics, and a React dashboard.

[![CI](https://github.com/gauravdev0011-code/EdgeSense/actions/workflows/ci.yml/badge.svg)](https://github.com/gauravdev0011-code/EdgeSense/actions/workflows/ci.yml)

## What it demonstrates

- **Concurrent systems:** 8 sensor streams are ingested through independent C++ threads and a thread-safe queue.
- **ML inference:** a PyTorch model is exported to ONNX and executed from the native C++ path with ONNX Runtime.
- **Real-time telemetry:** sensor features, anomaly scores, processing latency, CPU utilization, and memory RSS are streamed over WebSockets.
- **Observability:** the React dashboard exposes pipeline state, tail latency, model confidence, and process resource usage.
- **Evaluation:** 10K+ labeled sensor windows are evaluated with precision, recall, F1, false-positive rate, and false-negative rate.

## Architecture

```text
8 simulated sensor streams
        |
        v
Concurrent ingestion -> thread-safe queue -> timestamp synchronization
        |
        v
Feature extraction -> scaler -> ONNX Runtime inference
        |
        +--------------------------> WebSocket telemetry :9002
        |                              |
        |                              v
        |                         React dashboard
        |
        +--------------------------> Linux process metrics
                                      CPU + RSS
```

## Repository Layout

```text
EdgeSense/
├── engine/                 # C++20 sensor pipeline and inference path
├── server/                 # WebSocket telemetry server
├── frontend/               # React + Recharts dashboard
├── ml/                     # training, evaluation, ONNX export/benchmark
│   ├── data/sensor_data.csv
│   ├── train_model.py
│   ├── evaluate_model.py
│   ├── export_onnx.py
│   ├── benchmark_onnx.py
│   └── requirements.txt
├── third_party/            # pinned ONNX Runtime Linux dependency
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

The evaluation workflow operates on the repository's **10K+ labeled sensor windows** and reports accuracy, precision, recall, F1, false-positive rate, false-negative rate, and a confusion matrix.

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

The ONNX benchmark runs an **8-stream workload** and reports p50/p95/p99 inference latency and estimated sensor-vector throughput. Performance figures are environment-specific measurements and should be reproduced on the target machine before being quoted independently.

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

The C++ process starts the WebSocket server on port `9002` and broadcasts JSON telemetry containing sensor features, anomaly probability, anomaly status, pipeline processing time, CPU utilization, and memory RSS.

## Frontend

```bash
cd frontend
npm ci
npm run lint
npm run build
npm run dev
```

The React dashboard connects to `ws://localhost:9002` and displays live sensor signals, anomaly status, ONNX inference output, pipeline processing time, CPU utilization, memory RSS, and connection health.

## Engineering Details

### Concurrency

C++ threads process the simulated sensor streams concurrently. Readings enter a thread-safe queue before timestamp synchronization and feature extraction.

### Synchronization

Timestamp-based grouping keeps downstream feature extraction operating on coherent sensor windows rather than arbitrary arrival order.

### Inference

The PyTorch model is exported to ONNX. The C++ inference path applies the saved training scaler before passing the four-feature vector to ONNX Runtime.

### Process telemetry

On Linux, the runtime reports process CPU utilization from `getrusage()` deltas and resident memory from `/proc/self/status`. These values are emitted with each telemetry cycle and rendered by the dashboard.

### Telemetry

The C++ server maintains connected WebSocket clients and broadcasts JSON telemetry for each processing cycle. The React frontend consumes those messages through the browser WebSocket API.

## Validation and Benchmarking

| Metric | Workload / output |
|---|---:|
| Simulated sensor streams | **8** |
| Labeled sensor windows | **10K+** |
| ML evaluation | **Precision, recall, F1, FPR, FNR** |
| Runtime observability | **Throughput, p50, p95, p99, CPU, RSS** |

## CI

GitHub Actions builds the native C++ pipeline and validates the frontend with dependency installation, linting, and a production build on every push and pull request.

## Project Status

The core sensor-processing, ML, inference, WebSocket, process-observability, and dashboard path is implemented. Further work includes broader sensor models, stronger automated test coverage, and deployment hardening.

## Author

Gaurav Dev
