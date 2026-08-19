import time
from pathlib import Path

import numpy as np
import onnxruntime as ort


ML_DIR = Path(__file__).resolve().parent
ONNX_PATH = ML_DIR / "anomaly_model.onnx"

NUM_SENSORS = 8
NUM_WARMUP_RUNS = 100
NUM_BENCHMARK_RUNS = 1000


def percentile(values, percentile_value):
    return np.percentile(values, percentile_value)


def main():
    print("Loading ONNX model...")

    session = ort.InferenceSession(
        str(ONNX_PATH),
        providers=["CPUExecutionProvider"]
    )

    input_name = session.get_inputs()[0].name

    # Each row represents one sensor's four extracted features:
    #
    # temperature, vibration, current, acceleration
    #
    # Eight rows represent the eight sensor streams.
    sensor_features = np.array(
        [
            [70.0, 0.2, 10.0, 0.3],
            [71.0, 0.1, 10.5, 0.2],
            [69.5, 0.3, 9.8, 0.4],
            [70.5, 0.2, 10.2, 0.1],
            [70.2, 0.1, 10.1, 0.3],
            [69.8, 0.2, 9.9, 0.2],
            [70.7, 0.3, 10.4, 0.4],
            [70.1, 0.2, 10.0, 0.2],
        ],
        dtype=np.float32
    )

    print(f"Input: {input_name}")
    print(f"Sensor streams: {NUM_SENSORS}")
    print(f"Warmup runs: {NUM_WARMUP_RUNS}")
    print(f"Benchmark runs: {NUM_BENCHMARK_RUNS}")

    # Verify the expected workload size.
    if sensor_features.shape != (NUM_SENSORS, 4):
        raise ValueError(
            f"Expected ({NUM_SENSORS}, 4) input, "
            f"got {sensor_features.shape}"
        )

    # Warm up the ONNX Runtime session.
    for _ in range(NUM_WARMUP_RUNS):
        session.run(
            None,
            {input_name: sensor_features}
        )

    latencies_ms = []

    print("\nRunning 8-sensor benchmark...")

    for _ in range(NUM_BENCHMARK_RUNS):
        start = time.perf_counter()

        session.run(
            None,
            {input_name: sensor_features}
        )

        end = time.perf_counter()

        latency_ms = (end - start) * 1000.0
        latencies_ms.append(latency_ms)

    latencies = np.array(latencies_ms)

    print("\n=== EdgeSense 8-Sensor ONNX Runtime Benchmark ===")

    print(f"Sensor streams : {NUM_SENSORS}")
    print(f"Runs           : {len(latencies)}")
    print(f"Min            : {latencies.min():.4f} ms")
    print(f"Mean           : {latencies.mean():.4f} ms")
    print(f"p50            : {percentile(latencies, 50):.4f} ms")
    print(f"p95            : {percentile(latencies, 95):.4f} ms")
    print(f"p99            : {percentile(latencies, 99):.4f} ms")
    print(f"Max            : {latencies.max():.4f} ms")

    throughput = NUM_SENSORS / (latencies.mean() / 1000.0)

    print(f"\nEstimated sensor inference throughput: "
          f"{throughput:.2f} sensor vectors/second")

    print("\nBenchmark complete.")


if __name__ == "__main__":
    main()