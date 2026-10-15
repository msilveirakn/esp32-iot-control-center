#include <WiFi.h>
#include <WebServer.h>

// ========================================
// CONFIGURAÇÃO DO WI-FI
// ========================================

const char* ssid = "ESP32-IOT";
const char* password = "12345678";

// ========================================
// CONFIGURAÇÃO DO HARDWARE
// ========================================

const int LED_PIN = 2;

// ========================================
// SERVIDOR WEB
// ========================================

WebServer server(80);

// ========================================
// ESTADO DO LED
// ========================================

bool ledState = false;

// ========================================
// PÁGINA PRINCIPAL
// ========================================

void handleRoot() {

  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">

<head>

<meta charset="UTF-8">

<meta name="viewport"
      content="width=device-width, initial-scale=1.0">

<title>ESP32 IoT Control</title>

<style>

body {
    margin: 0;
    font-family: Arial, sans-serif;
    background: #111827;
    color: white;
    text-align: center;
}

.container {
    max-width: 500px;
    margin: auto;
    padding: 25px;
}

.card {
    background: #1f2937;
    border-radius: 15px;
    padding: 25px;
    margin-top: 20px;
}

h1 {
    margin-bottom: 5px;
}

.status {
    font-size: 20px;
    margin: 20px;
}

button {
    width: 100%;
    padding: 18px;
    margin-top: 10px;

    border: none;
    border-radius: 10px;

    font-size: 18px;
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

.info {
    margin-top: 25px;
    color: #9ca3af;
    font-size: 14px;
}

</style>

</head>

<body>

<div class="container">

<h1>ESP32 IoT Control</h1>

<p>Central de controle local</p>

<div class="card">

<h2>💡 LED</h2>

<div class="status">

STATUS_LED

</div>

<a href="/toggle">

<button class="BUTTON_CLASS">

BUTTON_TEXT

</button>

</a>

</div>

<div class="card">

<h2>📡 ESP32</h2>

<p>Rede: ESP32-IOT</p>

<p>IP: 192.168.4.1</p>

</div>

<div class="info">

Servidor web local<br>
Sem Internet

</div>

</div>

</body>

</html>
)rawliteral";

  if (ledState) {

    html.replace("STATUS_LED", "🟢 LED LIGADO");
    html.replace("BUTTON_CLASS", "off");
    html.replace("BUTTON_TEXT", "DESLIGAR LED");

  } else {

    html.replace("STATUS_LED", "🔴 LED DESLIGADO");
    html.replace("BUTTON_CLASS", "on");
    html.replace("BUTTON_TEXT", "LIGAR LED");

  }

  server.send(200, "text/html", html);
}

// ========================================
// LIGA / DESLIGA LED
// ========================================

void handleToggle() {

  ledState = !ledState;

  digitalWrite(LED_PIN, ledState ? HIGH : LOW);

  server.sendHeader("Location", "/");

  server.send(303);
}

// ========================================
// PÁGINA NÃO ENCONTRADA
// ========================================

void handleNotFound() {

  server.send(
    404,
    "text/plain",
    "Pagina nao encontrada"
  );

}

// ========================================
// SETUP
// ========================================

void setup() {

  Serial.begin(115200);

  // Configura LED
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  // ========================================
  // CRIA A REDE WI-FI
  // ========================================

  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid, password);

  // ========================================
  // MOSTRA INFORMAÇÕES NO SERIAL
  // ========================================

  Serial.println();
  Serial.println("==============================");
  Serial.println(" ESP32 IoT Control Center");
  Serial.println("==============================");

  Serial.print("Rede Wi-Fi: ");
  Serial.println(ssid);

  Serial.print("Senha: ");
  Serial.println(password);

  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.softAPIP());

  // ========================================
  // ROTAS DO SERVIDOR
  // ========================================

  server.on("/", handleRoot);

  server.on("/toggle", handleToggle);

  server.onNotFound(handleNotFound);

  // ========================================
  // INICIA SERVIDOR
  // ========================================

  server.begin();

  Serial.println("Servidor web iniciado!");
  Serial.println("==============================");
}

// ========================================
// LOOP
// ========================================

void loop() {

  server.handleClient();

}