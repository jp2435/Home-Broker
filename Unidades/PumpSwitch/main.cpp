/*
 * UNIT HEADER
 * ========================
 * Nome da Unidade: Pump Switch
 * Versão: v1.0.0
 * Microcontrolador: ESP8266
 * Autor: Jorge CP
 * Data: 05/07/2025
 * ========================
 */
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "{{ssid}}";
const char* password = "{{password}}";

ESP8266WebServer server(80);

const int relayPin = 5; // GPIO5 => D1
const int buttonPin = 14; // GPIO14 => D5

bool pumpState = false;

int estadoBotao;
int ultimoEstadoBotao=HIGH;
unsigned long ultimoDebounceTime=0;
unsigned long debounceDelay= 50; // 50ms pra debounc

IPAddress local_IP(192, 168, 0, 60);
IPAddress gateway(192, 168, 0, 62);
IPAddress subnet(255, 255, 255, 192);  // Subnet /26
IPAddress dns(8, 8, 8, 8);

void handleRoot();
void handleToggle();
void handleStatus();
void ChangeState();

void setup() {
  Serial.begin(9600);
  pinMode(relayPin,OUTPUT);
  pinMode(buttonPin,INPUT_PULLUP);
  digitalWrite(relayPin,LOW); // Desligar o Pump
  // Define IP fixo antes do WiFi.begin
  if (!WiFi.config(local_IP, gateway, subnet, dns)) {
    Serial.println("Falha ao configurar IP fixo");
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();

  int leitura = digitalRead(buttonPin);

  // Se o estado mudou
  if (leitura != ultimoEstadoBotao) {
    ultimoDebounceTime = millis(); // reinicia o contador de debounce
  }

  // Só considera mudança se passou o tempo de debounce
  if ((millis() - ultimoDebounceTime) > debounceDelay) {
    // aqui a leitura já estabilizou
    if (leitura != estadoBotao) {
      estadoBotao = leitura;

      if (estadoBotao == LOW) {
        Serial.println("Botão pressionado!");
        handleToggle();
      }
    }
  }

  ultimoEstadoBotao = leitura;
}

void ChangeState(){
  pumpState = !pumpState;
  Serial.println(pumpState);
  digitalWrite(relayPin,pumpState);
  delay(50);
}

void handleStatus() {
  String json = "{\"state\":\"" + String(pumpState ? "on" : "off") + "\"}";
  server.send(200, "application/json", json);
}

void handleToggle() {
  pumpState = !pumpState;
  digitalWrite(relayPin, pumpState ? HIGH : LOW); 
  server.send(200, "text/plain", "OK");
}
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html><html>
<head>
  <title>PumpSwitch</title>
</head>
<script>
  function updateState() {
    fetch('/status')
      .then(response => response.json())
      .then(data => {
        document.getElementById("estado").innerText = data.state == "on" ? "ON" : "OFF";
      })
      .catch(err => console.log("Erro:", err));
  }

  function toggle() {
    fetch('/toggle')
      .then(() => updateState());
  }

  setInterval(updateState, 5000); // chama a cada 5 s
  updateState(); // chama logo no início
</script>
<body>
  <h1>PumpSwitch</h1>
  <p>Estado: <strong id="estado">...</strong></p>
  <button onclick="toggle()">Toggle Pump</button>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}
