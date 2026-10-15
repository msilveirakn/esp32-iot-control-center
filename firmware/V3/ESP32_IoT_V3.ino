/*
  ESP32 IoT Control Center - V3
  Hardware: classic ESP32, DHT22, potentiometer, LED
  Wi-Fi: ESP32 access point (no router or Internet required)

  Pins:
    LED   -> GPIO 2
    DHT22 -> GPIO 4
    Potentiometer wiper -> GPIO 34
*/

#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <math.h>

// ---------- Wi-Fi access point ----------
const char* AP_SSID = "ESP32-IOT";
const char* AP_PASSWORD = "12345678";

// ---------- Hardware ----------
constexpr uint8_t LED_PIN = 2;
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t POT_PIN = 34;
constexpr uint32_t SENSOR_INTERVAL_MS = 2500;

#define DHT_TYPE DHT22

WebServer server(80);
DHT dht(DHT_PIN, DHT_TYPE);

// ---------- State ----------
bool ledState = false;
float temperatureC = NAN;
float humidityPct = NAN;
int potentiometerValue = 0;
uint32_t lastSensorReadMs = 0;

// ---------- Web interface ----------
const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 IoT Control Center - V3</title>
  <style>
    :root {
      color-scheme: dark;
      --bg: #101820;
      --panel: #1b2835;
      --panel2: #243444;
      --text: #eef5fb;
      --muted: #a9bac9;
      --accent: #55c2a3;
      --danger: #ff827e;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      padding: 24px 16px 40px;
      background: var(--bg);
      color: var(--text);
      font-family: system-ui, -apple-system, sans-serif;
    }
    main { max-width: 850px; margin: 0 auto; }
    header { margin-bottom: 24px; }
    h1 { margin: 0 0 8px; font-size: clamp(1.5rem, 4vw, 2.2rem); }
    p { color: var(--muted); line-height: 1.5; }
    .status {
      display: flex; align-items: center; gap: 10px;
      background: var(--panel); padding: 12px 14px; border-radius: 12px;
      margin: 18px 0;
    }
    .dot { width: 10px; height: 10px; border-radius: 50%; background: var(--accent); }
    .dot.offline { background: var(--danger); }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(210px, 1fr)); gap: 14px; }
    .card { background: var(--panel); border-radius: 16px; padding: 18px; }
    .label { color: var(--muted); font-size: .95rem; }
    .value { font-size: 2rem; font-weight: 700; margin: 10px 0; }
    .unit { color: var(--muted); font-size: 1rem; font-weight: 500; }
    .bar { height: 8px; background: var(--panel2); border-radius: 99px; overflow: hidden; }
    .bar > div { height: 100%; width: 0%; background: var(--accent); transition: width .2s ease; }
    button {
      border: 0; border-radius: 10px; padding: 12px 16px; font: inherit;
      font-weight: 700; cursor: pointer; background: var(--accent); color: #10231f;
    }
    button:disabled { opacity: .6; cursor: wait; }
    .secondary { background: var(--panel2); color: var(--text); }
    .actions { display: flex; flex-wrap: wrap; gap: 10px; margin-top: 14px; }
    #message { min-height: 1.5em; color: var(--muted); }
    footer { margin-top: 22px; color: var(--muted); font-size: .9rem; }
  </style>
</head>
<body>
<main>
  <header>
    <h1>ESP32 IoT Control Center</h1>
    <p>Versão 3 · Painel local de sensores e controle de saída</p>
  </header>

  <div class="status" role="status" aria-live="polite">
    <span id="statusDot" class="dot"></span>
    <span id="connectionStatus">Conectando ao ESP32...</span>
  </div>

  <section class="grid" aria-label="Leituras dos sensores">
    <article class="card">
      <div class="label">Temperatura</div>
      <div class="value"><span id="temperature">--</span> <span class="unit">°C</span></div>
      <p>Leitura do sensor DHT22</p>
    </article>
    <article class="card">
      <div class="label">Umidade relativa</div>
      <div class="value"><span id="humidity">--</span> <span class="unit">%</span></div>
      <p>Leitura do sensor DHT22</p>
    </article>
    <article class="card">
      <div class="label">Potenciômetro</div>
      <div class="value"><span id="potValue">--</span> <span class="unit">/ 4095</span></div>
      <div class="bar" aria-label="Posição do potenciômetro"><div id="potBar"></div></div>
      <p><span id="potPercent">--</span>% da escala aproximada</p>
    </article>
    <article class="card">
      <div class="label">Saída LED</div>
      <div class="value" id="ledState">--</div>
      <p>Controle digital do GPIO 2</p>
      <div class="actions">
        <button id="toggleButton" type="button">Alternar LED</button>
      </div>
      <p id="message" aria-live="polite"></p>
    </article>
  </section>

  <footer>
    Atualização automática a cada 2,5 segundos. Última atualização:
    <span id="lastUpdate">aguardando...</span>
  </footer>
</main>

<script>
  const $ = (id) => document.getElementById(id);
  const toggleButton = $("toggleButton");
  let requestInProgress = false;

  function displayNumber(value, decimals = 1) {
    if (value === null || value === undefined || !Number.isFinite(Number(value))) {
      return "--";
    }
    return Number(value).toFixed(decimals);
  }

  function updateDashboard(data) {
    $("temperature").textContent = displayNumber(data.temperature);
    $("humidity").textContent = displayNumber(data.humidity);
    $("potValue").textContent = displayNumber(data.pot, 0);

    const potPercent = Math.max(0, Math.min(100, (Number(data.pot) / 4095) * 100));
    $("potPercent").textContent = displayNumber(potPercent, 1);
    $("potBar").style.width = potPercent + "%";

    $("ledState").textContent = data.led ? "Ligado" : "Desligado";
    toggleButton.textContent = data.led ? "Desligar LED" : "Ligar LED";
    $("lastUpdate").textContent = new Date().toLocaleTimeString();

    $("statusDot").classList.remove("offline");
    $("connectionStatus").textContent = "Conectado ao ESP32";
  }

  async function loadData() {
    try {
      const response = await fetch("/data", { cache: "no-store" });
      if (!response.ok) throw new Error("HTTP " + response.status);
      const data = await response.json();
      updateDashboard(data);
    } catch (error) {
      $("statusDot").classList.add("offline");
      $("connectionStatus").textContent = "Sem comunicação com o ESP32";
    }
  }

  toggleButton.addEventListener("click", async () => {
    if (requestInProgress) return;
    requestInProgress = true;
    toggleButton.disabled = true;
    $("message").textContent = "Enviando comando...";

    try {
      const response = await fetch("/toggle", {
        method: "POST",
        cache: "no-store"
      });
      if (!response.ok) throw new Error("HTTP " + response.status);
      const result = await response.json();
      $("ledState").textContent = result.led ? "Ligado" : "Desligado";
      toggleButton.textContent = result.led ? "Desligar LED" : "Ligar LED";
      $("message").textContent = "Comando executado.";
      await loadData();
    } catch (error) {
      $("message").textContent = "Não foi possível alterar o LED.";
    } finally {
      requestInProgress = false;
      toggleButton.disabled = false;
    }
  });

  loadData();
  setInterval(loadData, 2500);
</script>
</body>
</html>
)rawliteral";

// ---------- Helpers ----------
void readSensors() {
  const uint32_t now = millis();

  if (now - lastSensorReadMs >= SENSOR_INTERVAL_MS) {
    lastSensorReadMs = now;

    // The DHT library can return NaN if a reading fails.
    humidityPct = dht.readHumidity();
    temperatureC = dht.readTemperature();
  }

  potentiometerValue = analogRead(POT_PIN);
}

void appendJsonFloat(String& json, const char* key, float value, uint8_t decimals) {
  json += '"';
  json += key;
  json += "\":";

  if (isnan(value)) {
    json += "null";
  } else {
    json += String(value, decimals);
  }
}

String buildStatusJson() {
  String json;
  json.reserve(120);

  json += "{\"led\":";
  json += (ledState ? "true" : "false");
  json += ",\"temperature\":";
  if (isnan(temperatureC)) {
    json += "null";
  } else {
    json += String(temperatureC, 1);
  }

  json += ",\"humidity\":";
  if (isnan(humidityPct)) {
    json += "null";
  } else {
    json += String(humidityPct, 1);
  }

  json += ",\"pot\":";
  json += String(potentiometerValue);
  json += "}";

  return json;
}

void handleRoot() {
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html; charset=utf-8", PAGE);
}

void handleData() {
  readSensors();
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", buildStatusJson());
}

void handleToggle() {
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);

  String response = "{\"led\":";
  response += (ledState ? "true" : "false");
  response += "}";

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", response);
}

void handleNotFound() {
  server.send(404, "text/plain; charset=utf-8", "404 - Rota não encontrada");
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  analogReadResolution(12);

  dht.begin();

  WiFi.mode(WIFI_AP);
  const bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);

  Serial.println();
  Serial.println("ESP32 IoT Control Center - V3");

  if (apStarted) {
    Serial.print("Rede Wi-Fi: ");
    Serial.println(AP_SSID);
    Serial.print("Acesse: http://");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("ERRO: não foi possível iniciar o ponto de acesso.");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.on("/toggle", HTTP_POST, handleToggle);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();
  readSensors();
}
