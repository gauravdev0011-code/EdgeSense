from pathlib import Path

import onnx
import torch
import torch.nn as nn


ML_DIR = Path(__file__).resolve().parent

MODEL_PATH = ML_DIR / "anomaly_model.pt"
ONNX_PATH = ML_DIR / "anomaly_model.onnx"


class AnomalyDetector(nn.Module):
    def __init__(self):
        super().__init__()

        self.network = nn.Sequential(
            nn.Linear(4, 16),
            nn.ReLU(),
            nn.Linear(16, 8),
            nn.ReLU(),
            nn.Linear(8, 1)
        )

    def forward(self, x):
        return self.network(x)


def main():
    print("Loading trained PyTorch model...")

    model = AnomalyDetector()

    state_dict = torch.load(
        MODEL_PATH,
        map_location="cpu"
    )

    model.load_state_dict(state_dict)
    model.eval()

    # Four sensor features:
    # temperature, vibration, current, acceleration
    example_input = torch.randn(1, 4)

    print("Exporting model to ONNX...")

    torch.onnx.export(
        model,
        example_input,
        ONNX_PATH,
        input_names=["sensor_features"],
        output_names=["anomaly_logit"],
        dynamic_axes={
            "sensor_features": {0: "batch_size"},
            "anomaly_logit": {0: "batch_size"}
        },
        opset_version=17,
        external_data=False
    )


    # Validate the exported ONNX model.
    onnx_model = onnx.load(ONNX_PATH)
    onnx.checker.check_model(onnx_model)

    print()
    print(f"ONNX model saved to: {ONNX_PATH}")
    print("ONNX model validation: PASSED")
    print()
    print("EdgeSense Day 6 ONNX export complete.")


if __name__ == "__main__":
    main()