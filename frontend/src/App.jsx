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

function App() {
  const [telemetry, setTelemetry] = useState(null);
  const [history, setHistory] = useState([]);
  const [connected, setConnected] = useState(false);

  useEffect(() => {
    const socket = new WebSocket("ws://localhost:9002");

    socket.onopen = () => {
      console.log("Connected to EdgeSense WebSocket");
      setConnected(true);
    };

    socket.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);

        setTelemetry(data);

        setHistory((previous) => {
          const next = [
            ...previous,
            {
              time: new Date().toLocaleTimeString(),
              temperature: data.temperature,
              vibration: data.vibration,
              current: data.current,
              acceleration: data.acceleration,
              probability: data.anomaly_probability,
            },
          ];

          return next.slice(-20);
        });
      } catch (error) {
        console.error("Invalid telemetry:", error);
      }
    };

    socket.onerror = (error) => {
      console.error("WebSocket error:", error);
      setConnected(false);
    };

    socket.onclose = () => {
      console.log("EdgeSense WebSocket disconnected");
      setConnected(false);
    };

    return () => {
      socket.close();
    };
  }, []);

  return (
    <div className="dashboard">
      <header className="header">
        <div>
          <h1>EdgeSense</h1>
          <p>Edge AI Sensor Monitoring Platform</p>
        </div>

        <div className={`status ${connected ? "online" : "offline"}`}>
          <span className="status-dot"></span>
          {connected ? "CONNECTED" : "DISCONNECTED"}
        </div>
      </header>

      <main>
        <section className="overview-grid">
          <div className="card">
            <span className="label">TEMPERATURE</span>
            <strong>
              {telemetry ? telemetry.temperature.toFixed(3) : "--"}
            </strong>
            <small>Normalized sensor value</small>
          </div>

          <div className="card">
            <span className="label">VIBRATION</span>
            <strong>
              {telemetry ? telemetry.vibration.toFixed(3) : "--"}
            </strong>
            <small>Normalized sensor value</small>
          </div>

          <div className="card">
            <span className="label">CURRENT</span>
            <strong>
              {telemetry ? telemetry.current.toFixed(3) : "--"}
            </strong>
            <small>Normalized sensor value</small>
          </div>

          <div className="card">
            <span className="label">ACCELERATION</span>
            <strong>
              {telemetry ? telemetry.acceleration.toFixed(3) : "--"}
            </strong>
            <small>Normalized sensor value</small>
          </div>
        </section>

        <section className="main-grid">
          <div className="panel chart-panel">
            <div className="panel-header">
              <div>
                <h2>Sensor Telemetry</h2>
                <p>Real-time normalized sensor readings</p>
              </div>
            </div>

            <div className="chart">
              <ResponsiveContainer width="100%" height="100%">
                <LineChart data={history}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="time" />
                  <YAxis />
                  <Tooltip />
                  <Line
                    type="monotone"
                    dataKey="temperature"
                    strokeWidth={2}
                    dot={false}
                  />
                  <Line
                    type="monotone"
                    dataKey="vibration"
                    strokeWidth={2}
                    dot={false}
                  />
                  <Line
                    type="monotone"
                    dataKey="current"
                    strokeWidth={2}
                    dot={false}
                  />
                  <Line
                    type="monotone"
                    dataKey="acceleration"
                    strokeWidth={2}
                    dot={false}
                  />
                </LineChart>
              </ResponsiveContainer>
            </div>
          </div>

          <div className="panel anomaly-panel">
            <h2>ML Inference</h2>
            <p className="panel-subtitle">ONNX anomaly detection</p>

            <div className="inference-row">
              <span>Anomaly Logit</span>
              <strong>
                {telemetry
                  ? telemetry.anomaly_logit.toFixed(3)
                  : "--"}
              </strong>
            </div>

            <div className="inference-row">
              <span>Anomaly Probability</span>
              <strong>
                {telemetry
                  ? `${(telemetry.anomaly_probability * 100).toFixed(1)}%`
                  : "--"}
              </strong>
            </div>

            <div
              className={`anomaly-result ${
                telemetry?.anomaly_detected ? "danger" : "safe"
              }`}
            >
              <span className="result-title">
                {telemetry
                  ? telemetry.anomaly_detected
                    ? "ANOMALY DETECTED"
                    : "SYSTEM NORMAL"
                  : "WAITING FOR DATA"}
              </span>

              <span className="result-description">
                {telemetry
                  ? telemetry.anomaly_detected
                    ? "The ML model classified the current telemetry as anomalous."
                    : "The ML model classified the current telemetry as normal."
                  : "Waiting for telemetry from the EdgeSense backend."}
              </span>
            </div>

            <div className="inference-row">
              <span>Processing Time</span>
              <strong>
                {telemetry
                  ? `${telemetry.processing_time_ms.toFixed(2)} ms`
                  : "--"}
              </strong>
            </div>
          </div>
        </section>

        <section className="panel probability-panel">
          <div className="panel-header">
            <div>
              <h2>Anomaly Probability</h2>
              <p>Model confidence over recent telemetry</p>
            </div>

            <strong className="probability-value">
              {telemetry
                ? `${(telemetry.anomaly_probability * 100).toFixed(1)}%`
                : "--"}
            </strong>
          </div>

          <div className="probability-chart">
            <ResponsiveContainer width="100%" height="100%">
              <LineChart data={history}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="time" />
                <YAxis domain={[0, 1]} />
                <Tooltip />
                <Line
                  type="monotone"
                  dataKey="probability"
                  strokeWidth={3}
                  dot={false}
                />
              </LineChart>
            </ResponsiveContainer>
          </div>
        </section>

        <footer>
          <span>EdgeSense Edge AI Pipeline</span>
          <span>
            WebSocket: {connected ? "Port 9002" : "Unavailable"}
          </span>
        </footer>
      </main>
    </div>
  );
}

export default App;