import { useEffect, useState } from "react";
import { Area, AreaChart, CartesianGrid, ResponsiveContainer, Tooltip, XAxis, YAxis } from "recharts";
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
  cpu_utilization_percent: 0,
  memory_mb: 0,
};

const streams = [
  ["TEMP-01", "Temperature"], ["TEMP-02", "Temperature"],
  ["VIB-01", "Vibration"], ["VIB-02", "Vibration"],
  ["CURR-01", "Current"], ["CURR-02", "Current"],
  ["ACC-01", "Acceleration"], ["ACC-02", "Acceleration"],
];

function App() {
  const [telemetry, setTelemetry] = useState(initialTelemetry);
  const [history, setHistory] = useState([]);
  const [connected, setConnected] = useState(false);
  const [lastUpdate, setLastUpdate] = useState(null);

  useEffect(() => {
    let socket;
    let timer;
    let disposed = false;

    const connect = () => {
      if (disposed) return;
      socket = new WebSocket("ws://localhost:9002");
      socket.onopen = () => setConnected(true);
      socket.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          const nextLatency = Number(data.processing_time_ms) || 0;
          setTelemetry({ ...initialTelemetry, ...data });
          setLastUpdate(new Date());
          setHistory((previous) => [...previous, {
            time: new Date().toLocaleTimeString([], { minute: "2-digit", second: "2-digit" }),
            temperature: Number(data.temperature) || 0,
            vibration: Number(data.vibration) || 0,
            current: Number(data.current) || 0,
            acceleration: Number(data.acceleration) || 0,
            latency: nextLatency,
          }].slice(-48));
        } catch {
          console.error("Invalid telemetry message");
        }
      };
      socket.onclose = () => {
        setConnected(false);
        if (!disposed) timer = window.setTimeout(connect, 2000);
      };
      socket.onerror = () => setConnected(false);
    };

    connect();
    return () => {
      disposed = true;
      window.clearTimeout(timer);
      socket?.close();
    };
  }, []);

  const probability = Math.max(0, Math.min(1, Number(telemetry.anomaly_probability) || 0));
  const latency = Number(telemetry.processing_time_ms) || 0;
  const cpu = Math.max(0, Number(telemetry.cpu_utilization_percent) || 0);
  const memory = Math.max(0, Number(telemetry.memory_mb) || 0);
  const anomaly = Boolean(telemetry.anomaly_detected);
  const maxLatency = history.length ? Math.max(...history.map((item) => item.latency)) : latency;

  return (
    <main className="app-shell">
      <div className="ambient ambient-one" /><div className="ambient ambient-two" />

      <header className="topbar">
        <div className="brand-block">
          <div className="brand-mark">ES</div>
          <div>
            <div className="brand-row"><h1>EdgeSense</h1><span className="version">EDGE AI</span></div>
            <p>Real-time sensor fusion &amp; anomaly detection</p>
          </div>
        </div>
        <div className="system-state">
          <span className={`state-dot ${connected ? "online" : "offline"}`} />
          <div><strong>{connected ? "LIVE PIPELINE" : "WAITING FOR PIPELINE"}</strong><span>{connected ? "WebSocket :9002" : "Reconnecting automatically"}</span></div>
        </div>
      </header>

      <section className="hero-grid">
        <article className={`hero-card ${anomaly ? "danger" : "healthy"}`}>
          <div className="hero-copy">
            <div className="section-kicker">MODEL DECISION</div>
            <div className="hero-status-row"><span className={`status-icon ${anomaly ? "danger" : "healthy"}`}>{anomaly ? "!" : "✓"}</span><h2>{anomaly ? "ANOMALY DETECTED" : "SYSTEM NORMAL"}</h2></div>
            <p>ONNX Runtime is evaluating the synchronized sensor window in real time.</p>
          </div>
          <div className="confidence"><span>ANOMALY PROBABILITY</span><strong>{(probability * 100).toFixed(1)}<small>%</small></strong><div className="confidence-track"><div style={{ width: `${probability * 100}%` }} /></div></div>
        </article>

        <article className="architecture-card">
          <div className="section-kicker">PIPELINE</div>
          <div className="pipeline-flow"><PipelineNode label="8 STREAMS" detail="C++20" /><i>→</i><PipelineNode label="FUSION" detail="Buffered" /><i>→</i><PipelineNode label="INFERENCE" detail="ONNX" /><i>→</i><PipelineNode label="TELEMETRY" detail="WebSocket" /></div>
          <div className="architecture-footer"><span>React dashboard</span><span>•</span><span>Live monitoring</span></div>
        </article>
      </section>

      <section className="metric-grid">
        <Metric label="TEMPERATURE" value={telemetry.temperature} unit="°C" />
        <Metric label="VIBRATION" value={telemetry.vibration} unit="g" />
        <Metric label="CURRENT" value={telemetry.current} unit="A" />
        <Metric label="ACCELERATION" value={telemetry.acceleration} unit="g" />
        <Metric label="INFERENCE" value={latency} unit="ms" accent="purple" />
        <Metric label="CPU" value={cpu} unit="%" accent="green" />
        <Metric label="MEMORY RSS" value={memory} unit="MB" accent="green" />
      </section>

      <section className="dashboard-grid">
        <article className="panel chart-panel">
          <PanelHeader kicker="LIVE TELEMETRY" title="Sensor signal history" meta={history.length ? `${history.length} samples buffered` : "Awaiting data"} />
          <div className="chart-wrap">{history.length > 1 ? <ResponsiveContainer width="100%" height="100%"><AreaChart data={history} margin={{ top: 8, right: 8, left: -18, bottom: 0 }}><defs><linearGradient id="signalFill" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stopColor="#5ee7ff" stopOpacity={0.24} /><stop offset="100%" stopColor="#5ee7ff" stopOpacity={0} /></linearGradient></defs><CartesianGrid stroke="#1c2940" vertical={false} /><XAxis dataKey="time" tickLine={false} axisLine={false} tick={{ fill: "#61718d", fontSize: 10 }} /><YAxis tickLine={false} axisLine={false} tick={{ fill: "#61718d", fontSize: 10 }} /><Tooltip contentStyle={{ background: "#0d1525", border: "1px solid #24334e", borderRadius: 10, color: "#eaf2ff" }} /><Area type="monotone" dataKey="temperature" stroke="#5ee7ff" fill="url(#signalFill)" strokeWidth={2} dot={false} name="Temperature" /><Area type="monotone" dataKey="vibration" stroke="#a78bfa" fill="none" strokeWidth={1.6} dot={false} name="Vibration" /><Area type="monotone" dataKey="current" stroke="#55e6a5" fill="none" strokeWidth={1.6} dot={false} name="Current" /><Area type="monotone" dataKey="acceleration" stroke="#f5c451" fill="none" strokeWidth={1.6} dot={false} name="Acceleration" /></AreaChart></ResponsiveContainer> : <div className="empty-state"><div className="pulse-ring" /><strong>Waiting for telemetry</strong><span>Start the C++ pipeline to stream sensor data.</span></div>}</div>
          <div className="chart-legend"><Legend color="#5ee7ff" label="Temperature" /><Legend color="#a78bfa" label="Vibration" /><Legend color="#55e6a5" label="Current" /><Legend color="#f5c451" label="Acceleration" /></div>
        </article>

        <article className="panel inference-card">
          <PanelHeader kicker="ML INFERENCE" title="Anomaly model" meta="PyTorch → ONNX Runtime" />
          <div className="model-score"><div className="score-ring" style={{ "--score": `${probability * 100}%` }}><div><strong>{(probability * 100).toFixed(0)}%</strong><span>confidence</span></div></div></div>
          <Detail label="Logit" value={(Number(telemetry.anomaly_logit) || 0).toFixed(3)} />
          <Detail label="Inference latency" value={`${latency.toFixed(2)} ms`} />
          <Detail label="Peak observed latency" value={`${maxLatency.toFixed(2)} ms`} />
          <Detail label="Input features" value="4 sensor features" />
        </article>
      </section>

      <section className="lower-grid">
        <article className="panel stream-panel">
          <PanelHeader kicker="SENSOR FABRIC" title="8-stream workload" meta="Concurrent simulated inputs" />
          <div className="stream-grid">{streams.map(([name, type]) => <div className="stream" key={name}><span className={`stream-indicator ${connected ? "active" : ""}`} /><div><strong>{name}</strong><span>{type}</span></div><b>{connected ? "ACTIVE" : "IDLE"}</b></div>)}</div>
        </article>

        <article className="panel health-panel">
          <PanelHeader kicker="RUNTIME HEALTH" title="Pipeline status" meta={lastUpdate ? `Updated ${lastUpdate.toLocaleTimeString()}` : "No telemetry yet"} />
          <Health label="WebSocket transport" value={connected ? "Connected" : "Offline"} healthy={connected} />
          <Health label="8-stream ingestion" value={connected ? "Running" : "Waiting"} healthy={connected} />
          <Health label="ML inference" value={connected ? "Online" : "Standby"} healthy={connected} />
          <Health label="Process telemetry" value={connected ? "CPU + RSS" : "Waiting"} healthy={connected} />
          <Health label="Anomaly state" value={anomaly ? "Alert" : "Normal"} healthy={!anomaly} />
        </article>
      </section>

      <footer className="footer"><span>EDGESENSE</span><span>•</span><span>C++20</span><span>•</span><span>PyTorch / ONNX Runtime</span><span>•</span><span>WebSocket</span><span>•</span><span>React</span></footer>
    </main>
  );
}

function Metric({ label, value, unit, accent = "cyan" }) { const n = Number(value) || 0; return <article className={`metric-card ${accent}`}><span>{label}</span><strong>{n.toFixed(2)}<small>{unit}</small></strong><div className="metric-line" /></article>; }
function PipelineNode({ label, detail }) { return <div className="pipeline-node"><strong>{label}</strong><span>{detail}</span></div>; }
function PanelHeader({ kicker, title, meta }) { return <div className="panel-header"><div><div className="section-kicker">{kicker}</div><h3>{title}</h3></div><span className="panel-meta">{meta}</span></div>; }
function Legend({ color, label }) { return <span className="legend-item"><i style={{ background: color }} />{label}</span>; }
function Detail({ label, value }) { return <div className="detail-row"><span>{label}</span><strong>{value}</strong></div>; }
function Health({ label, value, healthy }) { return <div className="health-row"><div><span className={`health-dot ${healthy ? "healthy" : "idle"}`} /><strong>{label}</strong></div><span className={healthy ? "health-value" : "idle-value"}>{value}</span></div>; }

export default App;
