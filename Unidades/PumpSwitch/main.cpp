/*
 * UNIT HEADER
 * ========================
 * Nome da Unidade: Pump Switch
 * Versão: v1.1.0
 * Microcontrolador: ESP8266
 * Autor: Jorge CP
 * Data: 05/07/2025
 * ========================
 */
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include "secrets.h"

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

const int relayPin = 5; // GPIO5 => D1
const int buttonPin = 14; // GPIO14 => D5

bool pumpState = false;

int estadoBotao;
int ultimoEstadoBotao=HIGH;
unsigned long ultimoDebounceTime=0;
unsigned long debounceDelay= 50; // 50ms pra debounc


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

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  httpUpdater.setup(&server,"/firmware",OTA_USER,OTA_PASS);

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
  <title>Pump Switch</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body { margin:0; font-family: Arial, sans-serif; }
    .header { background:#a8b5f0; padding:20px; text-align:center; font-size:28px; font-weight:bold; color:white; }
    .content { background:#4a63d9; min-height:calc(100vh - 60px); text-align:center; padding:40px 20px; }
    .status-label { font-size:24px; font-weight:bold; color:white; margin-bottom:10px; }
    .status { font-size:26px; font-weight:bold; margin-bottom:40px; }
    .button { background:#a8b5f0; border:none; border-radius:30px; padding:15px 30px; font-size:18px; font-weight:bold; color:#1c2b4a; cursor:pointer; }
  </style>
</head>
<body>
  <div class="header">Pump Switch</div>
  <div class="content">
    <div class="status-label">Estado</div>
    <div id="estado" class="status">...</div>
    <button class="button" onclick="toggle()">Ligar/Desligar</button>
  </div>

  <script>
    function updateState(){
      fetch('/status').then(res=>res.json()).then(data=>{
        let estado = document.getElementById("estado");
        if(data.state=="on"){
          estado.innerText="Ligado";
          estado.style.color="limegreen";
        }else{
          estado.innerText="Desligado";
          estado.style.color="red";
        }
      });
    }
    function toggle(){
      fetch('/toggle').then(()=>updateState());
    }
    setInterval(updateState, 1000);
    updateState();
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}
