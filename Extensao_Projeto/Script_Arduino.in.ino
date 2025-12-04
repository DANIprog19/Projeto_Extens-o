#include <WiFi.h>
#include <WebServer.h>

const char* ssid = " ";
const char* password = " ";

WebServer server(80);

const int sensorPin= 34;

int lerSensor(){
  int valor = analogRead(sensorPin);

  int percent = map(valor, 0, 4095, 0, 100);
  return percent;
}

void handleSensor(){
  int percent = lerSensor();
  String json = "{\"percent\": " + String(percent)+ "}";
  server.send(200, "application/json",json);
}

void setup(){
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println(" . ");
  }
  Serial.println("");
  Serial.println("Wifi conectado; " + WiFi.localIP().toString());

  server.on("/sensor", handleSensor);
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop(){
  server.handleClient();
}
