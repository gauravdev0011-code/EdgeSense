import { useEffect, useState } from "react";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
} from "recharts";
import "./App.css";

const initialTelemetry = {
  temperature: 0,
  vibration: 0,
  current: 0,
  acceleration: 0,
  anomaly_logit: 0,
  anomaly_probability: 0,
  anomaly_detected: false,
  processing_time_ms: 0,
};

function App() {
  const [telemetry, setTelemetry] = useState(initialTelemetry);
  const [history, setHistory] = useState([]);
  const [connected, setConnected] = useState(false);

  useEffect(() => {
    const socket = new WebSocket("ws://localhost:9002");

    socket.onopen = () => setConnected(true);

    socket.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);

        setTelemetry(data);

        setHistory((previous) => {
          const next = [
            ...previous,
            {
              time: new Date().toLocaleTimeString([], {
                minute: "2-digit",
                second: "2-digit",
              }),
              temperature: data.temperature,
              vibration: data.vibration,
              current: data.current,
              acceleration: data.acceleration,
            },
          ];

          return next.slice(-40);
        });
      } catch {
        console.error("Invalid telemetry message");
      }
    };

    socket.onclose = () => setConnected(false);
    socket.onerror = () => setConnected(false);

    return () => socket.close();
  }, []);

  const probability = telemetry.anomaly_probability;
  const status = telemetry.anomaly_detected ? "ANOMALY" : "NORMAL";

  return (
    <main className="dashboard">
      <header className="header">
        <div>
          <p className="eyebrow">EDGE AI MONITORING</p>
          <h1>EdgeSense</h1>
          <p className="subtitle">
            Real-time sensor fusion and anomaly detection
          </p>
        </div>

        <div className={`connection ${connected ? "online" : "offline"}`}>
          <span />
          {connected ? "CONNECTED" : "OFFLINE"}
        </div>
      </header>

      <section
        className={`status-card ${
          telemetry.anomaly_detected ? "alert" : ""
        }`}
      >
        <div>
          <p className="label">SYSTEM STATUS</p>
          <h2>{status}</h2>
          <p className="muted">
            Anomaly probability{" "}
            <strong>{probability.toFixed(3)}</strong>
          </p>
        </div>

        <div className="score">
          {(probability * 100).toFixed(1)}%
        </div>
      </section>

      <section className="metrics">
        <Metric title="Temperature" value={telemetry.temperature} />
        <Metric title="Vibration" value={telemetry.vibration} />
        <Metric title="Current" value={telemetry.current} />
        <Metric title="Acceleration" value={telemetry.acceleration} />
      </section>

      <section className="content-grid">
        <article className="panel telemetry-panel">
          <div className="panel-heading">
            <div>
              <p className="eyebrow">SENSOR TELEMETRY</p>
              <h3>Live sensor streams</h3>
            </div>

            <span className="live">
              <span />
              LIVE
            </span>
          </div>

          <div className="chart">
            {history.length > 1 ? (
              <ResponsiveContainer width="100%" height="100%">
                <LineChart data={history}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="time" />
                  <YAxis />
                  <Tooltip />

                  <Line
                    type="monotone"
                    dataKey="temperature"
                    stroke="#60a5fa"
                    strokeWidth={2}
                    dot={false}
                  />

                  <Line
                    type="monotone"
                    dataKey="vibration"
                    stroke="#a78bfa"
                    strokeWidth={2}
                    dot={false}
                  />

                  <Line
                    type="monotone"
                    dataKey="current"
                    stroke="#34d399"
                    strokeWidth={2}
                    dot={false}
                  />

                  <Line
                    type="monotone"
                    dataKey="acceleration"
                    stroke="#fbbf24"
                    strokeWidth={2}
                    dot={false}
                  />
                </LineChart>
              </ResponsiveContainer>
            ) : (
              <div className="chart-empty">
                Waiting for sensor history...
              </div>
            )}
          </div>

          <div className="legend">
            <span>
              <i className="temperature" />
              Temperature
            </span>

            <span>
              <i className="vibration" />
              Vibration
            </span>

            <span>
              <i className="current" />
              Current
            </span>

            <span>
              <i className="acceleration" />
              Acceleration
            </span>
          </div>
        </article>

        <article className="panel inference-panel">
          <p className="eyebrow">ML INFERENCE</p>
          <h3>ONNX anomaly detection</h3>

          <div className="inference-row">
            <span>Anomaly logit</span>
            <strong>
              {telemetry.anomaly_logit.toFixed(3)}
            </strong>
          </div>

          <div className="inference-row">
            <span>Probability</span>
            <strong>
              {(probability * 100).toFixed(1)}%
            </strong>
          </div>

          <div
            className={`detection ${
              telemetry.anomaly_detected
                ? "detected"
                : "normal"
            }`}
          >
            <strong>{status}</strong>

            <span>
              {telemetry.anomaly_detected
                ? "Model flagged the current sensor window."
                : "Current sensor window is within normal range."}
            </span>
          </div>
        </article>
      </section>

      <section className="performance">
        <Stat
          label="PROCESSING TIME"
          value={`${telemetry.processing_time_ms.toFixed(1)} ms`}
        />

        <Stat
          label="SENSOR STREAMS"
          value="8"
        />

        <Stat
          label="HISTORY BUFFER"
          value={`${history.length}/40`}
        />
      </section>

      <footer>
        EdgeSense • C++20 • ONNX Runtime • WebSockets
      </footer>
    </main>
  );
}

function Metric({ title, value }) {
  return (
    <article className="metric">
      <p>{title}</p>

      <strong>{value.toFixed(3)}</strong>

      <span>Normalized sensor value</span>
    </article>
  );
}

function Stat({ label, value }) {
  return (
    <div>
      <span>{label}</span>
      <strong>{value}</strong>
    </div>
  );
}

export default App;