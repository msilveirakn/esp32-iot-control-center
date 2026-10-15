
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// ========================================
// 1. CONFIGURACAO DO WI-FI
// ========================================

const char* ssid = "ESP32-IOT";
const char* password = "12345678";

// ========================================
// 2. PINOS
// ========================================

const int LED_PIN = 2;
const int DHT_PIN = 4;
const int POT_PIN = 34;

// ========================================
// 3. SENSOR DHT22
// ========================================

#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

// ========================================
// 4. SERVIDOR WEB
// ========================================

WebServer server(80);

// ========================================
// 5. VARIAVEIS DE ESTADO
// ========================================

bool ledState = false;

float temperature = NAN;
float humidity = NAN;

int potValue = 0;

unsigned long lastSensorRead = 0;
const unsigned long SENSOR_INTERVAL = 2500;

// ========================================
// 6. PAGINA HTML
// ========================================

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport"
      content="width=device-width, initial-scale=1.0">

<title>ESP32 IoT Control - V2</title>

<style>
* {
  box-sizing: border-box;
}

body {
  margin: 0;
  padding: 20px;
  font-family: Arial, sans-serif;
  background: #111827;
  color: white;
}

.container {
  max-width: 600px;
  margin: auto;
}

h1 {
  text-align: center;
  margin-bottom: 5px;
}

.subtitle {
  text-align: center;
  color: #9ca3af;
  margin-bottom: 25px;
}

.card {
  background: #1f2937;
  border-radius: 14px;
  padding: 20px;
  margin-bottom: 16px;
}

.value {
  font-size: 30px;
  font-weight: bold;
  margin: 12px 0;
}

.label {
  color: #9ca3af;
}

button {
  width: 100%;
  border: none;
  border-radius: 9px;
  padding: 15px;
  margin-top: 10px;
  font-size: 17px;
  font-weight: bold;
  cursor: pointer;
}

.on {
  background: #22c55e;
  color: white;
}

.off {
  background: #ef4444;
  color: white;
}

.small {
  color: #9ca3af;
  font-size: 13px;
  text-align: center;
}

.bar {
  height: 12px;
  background: #374151;
  border-radius: 10px;
  overflow: hidden;
}

#potBar {
  height: 100%;
  width: 0%;
  background: #38bdf8;
  transition: width 0.2s;
}
</style>
</head>

<body>
<div class="container">

<h1>ESP32 IoT Control</h1>
<p class="subtitle">V2 - Monitoramento de sensores</p>

<div class="card">
  <h2>💡 Controle do LED</h2>
  <p id="ledStatus" class="value">Carregando...</p>
  <button id="ledButton" class="on"
          onclick="toggleLED()">Carregando...</button>
</div>

<div class="card">
  <h2>🌡️ Temperatura</h2>
  <p id="temperature" class="value">-- °C</p>
  <p class="label">Sensor DHT22</p>
</div>

<div class="card">
  <h2>💧 Umidade do ar</h2>
  <p id="humidity" class="value">-- %</p>
  <p class="label">Sensor DHT22</p>
</div>

<div class="card">
  <h2>🎚️ Potenciômetro</h2>
  <p id="potValue" class="value">-- / 4095</p>
  <div class="bar">
    <div id="potBar"></div>
  </div>
  <p id="potPercent" class="label">0%</p>
  <p class="small">Leitura ADC de 12 bits</p>
</div>

<div class="card">
  <h2>📡 Conexao</h2>
  <p>Rede: ESP32-IOT</p>
  <p>IP: 192.168.4.1</p>
  <p id="connection">Aguardando dados...</p>
</div>

<p class="small">
  Atualizacao automatica a cada 2,5 segundos.
  Servidor local, sem Internet.
</p>

</div>

<script>
async function loadData() {
  try {
    const response = await fetch('/data', {
      cache: 'no-store'
    });

    if (!response.ok) {
      throw new Error('Erro HTTP');
    }

    const data = await response.json();

    document.getElementById('ledStatus').textContent =
      data.led ? '🟢 LIGADO' : '🔴 DESLIGADO';

    const button = document.getElementById('ledButton');
    button.textContent = data.led
      ? 'DESLIGAR LED'
      : 'LIGAR LED';

    button.className = data.led ? 'off' : 'on';

    document.getElementById('temperature').textContent =
      data.temperature === null
        ? 'Falha na leitura'
        : data.temperature.toFixed(1) + ' °C';

    document.getElementById('humidity').textContent =
      data.humidity === null
        ? 'Falha na leitura'
        : data.humidity.toFixed(1) + ' %';

    document.getElementById('potValue').textContent =
      data.pot + ' / 4095';

    const percent = (data.pot / 4095) * 100;

    document.getElementById('potPercent').textContent =
      percent.toFixed(1) + '%';

    document.getElementById('potBar').style.width =
      percent + '%';

    document.getElementById('connection').textContent =
      'ESP32 conectado; dados recebidos.';

  } catch (error) {
    document.getElementById('connection').textContent =
      'Falha ao comunicar com o ESP32.';
  }
}

async function toggleLED() {
  try {
    await fetch('/toggle', { method: 'POST' });
    await loadData();
  } catch (error) {
    document.getElementById('connection').textContent =
      'Falha ao controlar o LED.';
  }
}

loadData();
setInterval(loadData, 2500);
</script>

</body>
</html>
)rawliteral";

// ========================================
// 7. LEITURA DOS SENSORES
// ========================================

void readSensors() {
  unsigned long now = millis();

  if (now - lastSensorRead < SENSOR_INTERVAL) {
    return;
  }

  lastSensorRead = now;

  float newTemperature = dht.readTemperature();
  float newHumidity = dht.readHumidity();

  // Atualiza apenas se ambas as leituras forem validas.
  if (!isnan(newTemperature) && !isnan(newHumidity)) {
    temperature = newTemperature;
    humidity = newHumidity;
  } else {
    temperature = NAN;
    humidity = NAN;
    Serial.println("Erro ao ler o DHT22.");
  }

  // Leitura do ADC: ESP32 classico, 12 bits.
  potValue = analogRead(POT_PIN);

  Serial.print("Temperatura: ");
  if (isnan(temperature)) {
    Serial.print("Erro");
  } else {
    Serial.print(temperature);
    Serial.print(" C");
  }

  Serial.print(" | Umidade: ");
  if (isnan(humidity)) {
    Serial.print("Erro");
  } else {
    Serial.print(humidity);
    Serial.print(" %");
  }

  Serial.print(" | Potenciometro: ");
  Serial.println(potValue);
}

// ========================================
// 8. PAGINA PRINCIPAL
// ========================================

void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", PAGE);
}

// ========================================
// 9. DADOS EM JSON
// ========================================

void handleData() {
  String json = "{";

  json += "\"led\":";
  json += (ledState ? "true" : "false");

  json += ",\"temperature\":";
  json += (isnan(temperature) ? "null" : String(temperature, 1));

  json += ",\"humidity\":";
  json += (isnan(humidity) ? "null" : String(humidity, 1));

  json += ",\"pot\":";
  json += String(potValue);

  json += "}";

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

// ========================================
// 10. CONTROLE DO LED
// ========================================

void handleToggle() {
  ledState = !ledState;

  digitalWrite(LED_PIN, ledState ? HIGH : LOW);

  server.send(200, "text/plain", ledState ? "LED ligado" : "LED desligado");
}

// ========================================
// 11. ROTAS DESCONHECIDAS
// ========================================

void handleNotFound() {
  server.send(404, "text/plain", "Pagina nao encontrada");
}

// ========================================
// 12. SETUP
// ========================================

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(POT_PIN, INPUT);
  analogReadResolution(12);

  dht.begin();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("ESP32 IoT Control Center V2");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.on("/toggle", HTTP_POST, handleToggle);
  server.onNotFound(handleNotFound);

  server.begin();

  Serial.println("Servidor iniciado.");
}

// ========================================
// 13. LOOP
// ========================================

void loop() {
  server.handleClient();
  readSensors();
}