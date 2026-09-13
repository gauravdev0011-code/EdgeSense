# EdgeSense — Real-Time Edge AI Sensor Fusion & Anomaly Detection

EdgeSense is a C++20 edge-AI pipeline that simulates eight concurrent sensor streams, synchronizes readings into windows, extracts features, runs an ONNX Runtime anomaly detector, and publishes live telemetry to a React dashboard over WebSockets.

The repository is organized to make the data path explicit:

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

## Resume-Aligned Capabilities

- Multithreaded C++20 pipeline for **8 simulated sensor streams**
- Thread-safe buffering and timestamp-based synchronization
- Feature extraction for temperature, vibration, current, and acceleration
- PyTorch training with train/test split and standardized inputs
- ONNX export and ONNX Runtime CPU inference
- WebSocket telemetry on `ws://localhost:9002`
- React/Vite dashboard with live sensor charts and anomaly status
- Dataset evaluation with precision, recall, F1, false-positive rate, and false-negative rate
- Inference benchmark reporting p50/p95/p99 latency and throughput

## Repository Layout

```text
EdgeSense/
├── engine/
│   ├── include/
│   └── src/
├── server/
│   ├── include/
│   └── src/
├── frontend/
│   ├── src/
│   └── public/
├── ml/
│   ├── data/sensor_data.csv
│   ├── train_model.py
│   ├── evaluate_model.py
│   ├── export_onnx.py
│   ├── benchmark_onnx.py
│   └── requirements.txt
├── third_party/
│   └── onnxruntime-linux-x64-1.29.0/
├── CMakeLists.txt
└── README.md
```

## ML Pipeline

The dataset contains four sensor features and a binary anomaly label:

- temperature
- vibration
- current
- acceleration
- label

Training uses `StandardScaler`, so the same preprocessing parameters must be used during C++ inference. The training script therefore writes both a PyTorch scaler artifact and a plain-text `ml/scaler.txt` consumed by the C++ runtime.

### Install Python dependencies

```bash
cd ml
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

### Train and evaluate

```bash
python train_model.py
python evaluate_model.py
```

The evaluation reports the number of labeled windows plus accuracy, precision, recall, F1, false-positive rate, false-negative rate, and the confusion matrix.

### Export to ONNX

```bash
python export_onnx.py
```

The export script validates the generated ONNX graph before the C++ runtime uses it.

### Benchmark ONNX Runtime

```bash
python benchmark_onnx.py
```

The benchmark runs an 8-stream workload and reports p50/p95/p99 latency and estimated sensor-vector throughput.

Benchmark numbers are hardware- and workload-dependent. Do not present a single latency or throughput value as universal performance.

## Native Build

EdgeSense uses C++20, CMake, WebSocket++, standalone Asio, and the vendored ONNX Runtime package.

On Debian/Ubuntu, install the WebSocket dependencies first:

```bash
sudo apt update
sudo apt install -y build-essential cmake libasio-dev libwebsocketpp-dev
```

Build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Before running the executable, make sure these generated ML artifacts exist:

```text
ml/anomaly_model.onnx
ml/scaler.txt
```

Then run from the repository root:

```bash
./build/edgesense
```

The C++ process starts the WebSocket server on port `9002` and continuously broadcasts JSON telemetry containing sensor features, anomaly probability, anomaly status, and pipeline processing time.

## Frontend

```bash
cd frontend
npm install
npm run dev
```

The dashboard connects to `ws://localhost:9002` and displays:

- live temperature, vibration, current, and acceleration streams
- anomaly probability and detection state
- ONNX inference output
- pipeline processing time
- connection status

## Engineering Details

### Concurrency

Each sensor stream is sampled concurrently using C++ threads. Readings enter a thread-safe queue before synchronization and feature extraction.

### Synchronization

Readings are grouped using timestamps so downstream feature extraction operates on a coherent sensor window rather than arbitrary arrival order.

### Inference

The trained PyTorch model is exported to ONNX. The C++ `InferenceEngine` applies the saved training scaler before passing the four-feature vector to ONNX Runtime. This keeps training and deployed inference preprocessing consistent.

### Telemetry

The C++ server maintains connected WebSocket clients and broadcasts one JSON telemetry message per processing cycle. The React frontend consumes those messages directly through the browser WebSocket API.

## Project Status

The core sensor-processing, ML, inference, WebSocket, and dashboard path is implemented. The remaining work is primarily deeper validation, broader sensor models, stronger test coverage, and deployment hardening.

## Author

Gaurav Dev
