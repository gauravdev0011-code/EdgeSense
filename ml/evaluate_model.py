from pathlib import Path

import pandas as pd
import torch
import torch.nn as nn

from sklearn.metrics import (
    accuracy_score,
    confusion_matrix,
    f1_score,
    precision_score,
    recall_score,
)


ML_DIR = Path(__file__).resolve().parent
DATA_PATH = ML_DIR / "data" / "sensor_data.csv"
MODEL_PATH = ML_DIR / "anomaly_model.pt"
SCALER_TEXT_PATH = ML_DIR / "scaler.txt"
FEATURES = ["temperature", "vibration", "current", "acceleration"]


class AnomalyDetector(nn.Module):
    def __init__(self):
        super().__init__()
        self.network = nn.Sequential(
            nn.Linear(4, 16),
            nn.ReLU(),
            nn.Linear(16, 8),
            nn.ReLU(),
            nn.Linear(8, 1),
        )

    def forward(self, x):
        return self.network(x)


def load_scaler():
    lines = SCALER_TEXT_PATH.read_text(encoding="utf-8").splitlines()
    if len(lines) != 2:
        raise ValueError("scaler.txt must contain one line of means and one line of scales")

    mean = [float(value) for value in lines[0].split()]
    scale = [float(value) for value in lines[1].split()]

    if len(mean) != len(FEATURES) or len(scale) != len(FEATURES):
        raise ValueError("scaler.txt does not contain four feature parameters")
    if any(value == 0.0 for value in scale):
        raise ValueError("scaler.txt contains a zero scale")

    return mean, scale


def main():
    data = pd.read_csv(DATA_PATH)
    mean, scale = load_scaler()

    normalized = (data[FEATURES].values - mean) / scale

    model = AnomalyDetector()
    model.load_state_dict(torch.load(MODEL_PATH, map_location="cpu"))
    model.eval()

    inputs = torch.tensor(normalized, dtype=torch.float32)
    labels = data["label"].values

    with torch.no_grad():
        probabilities = torch.sigmoid(model(inputs)).numpy().ravel()

    predictions = (probabilities >= 0.5).astype(int)
    tn, fp, fn, tp = confusion_matrix(labels, predictions, labels=[0, 1]).ravel()

    accuracy = accuracy_score(labels, predictions)
    precision = precision_score(labels, predictions, zero_division=0)
    recall = recall_score(labels, predictions, zero_division=0)
    f1 = f1_score(labels, predictions, zero_division=0)
    fpr = fp / (fp + tn) if (fp + tn) else 0.0
    fnr = fn / (fn + tp) if (fn + tp) else 0.0

    print("=== EdgeSense Dataset Evaluation ===")
    print(f"Labeled windows : {len(data)}")
    print(f"Accuracy        : {accuracy:.4f}")
    print(f"Precision       : {precision:.4f}")
    print(f"Recall          : {recall:.4f}")
    print(f"F1              : {f1:.4f}")
    print(f"False-positive  : {fpr:.4f}")
    print(f"False-negative  : {fnr:.4f}")
    print(f"Confusion       : TN={tn}, FP={fp}, FN={fn}, TP={tp}")


if __name__ == "__main__":
    main()
