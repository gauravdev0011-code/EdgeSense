# EdgeSense

Real-time edge AI sensor fusion and anomaly detection system.

## Planned Stack

- C++20
- Python
- PyTorch
- ONNX Runtime
- Linux
- CMake
- React
- JavaScript
- WebSockets

## Status

Under active development.

## Current Pipeline

EdgeSense currently implements a real-time C++20 sensor processing pipeline:

Sensor Streams
→ Thread-Safe Queues
→ Sensor Synchronization
→ Feature Extraction
→ ML-Ready Feature Vectors

### Current Capabilities

- C++20 multithreaded sensor simulation
- 8+ synchronized sensor streams
- Thread-safe buffering and queue management
- Timestamp-based sensor synchronization
- Feature extraction from sensor windows
- ML-ready feature generation
- Synthetic labeled sensor dataset generation
- Pipeline processing-time measurement
- CMake-based Linux/WSL build

### Machine Learning Pipeline

The ML pipeline uses Python-generated sensor data containing:

- Temperature
- Vibration
- Current
- Acceleration
- Anomaly label

The next stage is to train a PyTorch anomaly-detection model and export it to ONNX for low-latency inference with ONNX Runtime.