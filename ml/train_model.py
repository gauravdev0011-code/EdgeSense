import os

import pandas as pd
import torch
import torch.nn as nn

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score, precision_score, recall_score, f1_score


DATA_PATH = "data/sensor_data.csv"
MODEL_PATH = "anomaly_model.pt"
SCALER_PATH = "scaler.pt"


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
    print("Loading sensor dataset...")

    data = pd.read_csv(DATA_PATH)

    features = [
        "temperature",
        "vibration",
        "current",
        "acceleration"
    ]

    X = data[features].values
    y = data["label"].values

    print(f"Samples: {len(data)}")
    print(f"Features: {len(features)}")
    print(f"Anomalies: {int(y.sum())}")
    print(f"Normal samples: {int(len(y) - y.sum())}")

    # Split dataset into training and testing sets.
    X_train, X_test, y_train, y_test = train_test_split(
        X,
        y,
        test_size=0.2,
        random_state=42,
        stratify=y
    )

    # Normalize the sensor values.
    scaler = StandardScaler()

    X_train = scaler.fit_transform(X_train)
    X_test = scaler.transform(X_test)

    # Convert NumPy arrays to PyTorch tensors.
    X_train = torch.tensor(X_train, dtype=torch.float32)
    X_test = torch.tensor(X_test, dtype=torch.float32)

    y_train = torch.tensor(
        y_train,
        dtype=torch.float32
    ).reshape(-1, 1)

    y_test = torch.tensor(
        y_test,
        dtype=torch.float32
    ).reshape(-1, 1)

    model = AnomalyDetector()

    loss_function = nn.BCEWithLogitsLoss()

    optimizer = torch.optim.Adam(
        model.parameters(),
        lr=0.001
    )

    epochs = 100

    print("\nTraining model...")

    for epoch in range(epochs):

        model.train()

        predictions = model(X_train)

        loss = loss_function(
            predictions,
            y_train
        )

        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

        if (epoch + 1) % 10 == 0:
            print(
                f"Epoch {epoch + 1:3d}/{epochs} "
                f"Loss: {loss.item():.4f}"
            )

    # Evaluate model.
    model.eval()

    with torch.no_grad():
        logits = model(X_test)

        probabilities = torch.sigmoid(logits)

        predictions = (
            probabilities >= 0.5
        ).int()

    y_true = y_test.int()

    accuracy = accuracy_score(
        y_true,
        predictions
    )

    precision = precision_score(
        y_true,
        predictions,
        zero_division=0
    )

    recall = recall_score(
        y_true,
        predictions,
        zero_division=0
    )

    f1 = f1_score(
        y_true,
        predictions,
        zero_division=0
    )

    print("\n--- Model Evaluation ---")

    print(f"Accuracy : {accuracy:.4f}")
    print(f"Precision: {precision:.4f}")
    print(f"Recall   : {recall:.4f}")
    print(f"F1 Score : {f1:.4f}")

    # Save model weights.
    torch.save(
        model.state_dict(),
        MODEL_PATH
    )

    # Save normalization parameters.
    torch.save(
        {
            "mean": scaler.mean_,
            "scale": scaler.scale_,
            "features": features
        },
        SCALER_PATH
    )

    print("\nSaved:")
    print(f"  {MODEL_PATH}")
    print(f"  {SCALER_PATH}")

    print("\nEdgeSense Day 5 training complete.")


if __name__ == "__main__":
    main()