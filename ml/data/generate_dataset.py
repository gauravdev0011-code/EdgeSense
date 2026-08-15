import csv
import random


OUTPUT_FILE = "data/sensor_data.csv"
NUM_SAMPLES = 10000


def generate_normal_sample():
    temperature = random.gauss(70.0, 2.0)
    vibration = random.gauss(0.0, 0.5)
    current = random.gauss(10.0, 1.0)
    acceleration = random.gauss(0.0, 0.8)

    return temperature, vibration, current, acceleration, 0


def generate_anomaly_sample():
    anomaly_type = random.choice(
        ["temperature", "vibration", "current", "acceleration"]
    )

    temperature = random.gauss(70.0, 2.0)
    vibration = random.gauss(0.0, 0.5)
    current = random.gauss(10.0, 1.0)
    acceleration = random.gauss(0.0, 0.8)

    if anomaly_type == "temperature":
        temperature += random.choice([-1, 1]) * random.uniform(8.0, 15.0)

    elif anomaly_type == "vibration":
        vibration += random.choice([-1, 1]) * random.uniform(3.0, 6.0)

    elif anomaly_type == "current":
        current += random.choice([-1, 1]) * random.uniform(4.0, 8.0)

    elif anomaly_type == "acceleration":
        acceleration += random.choice([-1, 1]) * random.uniform(4.0, 7.0)

    return temperature, vibration, current, acceleration, 1


def main():
    random.seed(42)

    with open(OUTPUT_FILE, "w", newline="") as file:
        writer = csv.writer(file)

        writer.writerow(
            [
                "temperature",
                "vibration",
                "current",
                "acceleration",
                "label",
            ]
        )

        normal_samples = NUM_SAMPLES // 2
        anomaly_samples = NUM_SAMPLES - normal_samples

        for _ in range(normal_samples):
            writer.writerow(generate_normal_sample())

        for _ in range(anomaly_samples):
            writer.writerow(generate_anomaly_sample())

    print(f"Generated {NUM_SAMPLES} sensor windows.")
    print(f"Saved dataset to {OUTPUT_FILE}")


if __name__ == "__main__":
    main()