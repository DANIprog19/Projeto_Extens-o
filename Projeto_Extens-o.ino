#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#define PINO_DHT 14
#define TIPO_DHT DHT11
#define PINO_MQ2 34
#define PINO_BUZZER 27

DHT sensor_dht(PINO_DHT, TIPO_DHT);
WebServer servidor(80);

const char site_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<title>Safe-Home</title>

<link href="https://fonts.googleapis.com/css2?family=Poppins:wght@400;600;700&display=swap" rel="stylesheet">

<style>
body{
    font-family: 'Poppins', sans-serif;
    background-color: #f5f5f5;
    margin: 0;
    padding: 20px;
    text-align: center;
}

h1.titulo{
    font-size: 36px;
    color: #333;
    margin-bottom: 30px;
}

.card-container{
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 20px;
}

.card{
    border: 1px solid #ccc;
    border-radius: 20px;
    box-shadow: 0 2px 8px rgba(0,0,0,0.2);
    padding: 50px 80px;
    width: 600px;
    margin: 0 auto;
    background-color: white;
    display: flex;
    flex-direction: column;
    align-items: center; 
}

.termometro-lado{
    display: flex; 
    flex-direction: row;
    justify-content: center; 
    gap: 80px; 
    align-items: flex-start; 
    margin-top: 20px;
    width: 100%;
}

.termometro-bg, .temperatura-bg, .umidade-bg{
    position: relative;
    width: 35px;
    height: 70%; 
    border-radius: 18px;
    background-color: #e5e7eb;
    overflow: hidden;
    border: 2px solid #ccc;
}

.termometro-fill, .temperatura-fill, .umidade-fill{
    position: absolute;
    bottom: 0;
    width: 100%;
    height: 0;
    background-color: #00bfff;
    border-radius: 15px 15px 0 0;
    transition: height 0.5s ease-in-out;
}

.termometro-container, .temperatura-container, .umidade-container{
    width: 80px;
    height: 280px; 
    display: flex;
    flex-direction: column;
    align-items: center; 
    justify-content: space-between; 
}

#alerta{
    display: none;
    background-color: #fbbf24;
    width: 80%;
    margin: 25px auto 0 auto;
    padding: 15px;
    border-radius: 10px;
    font-weight: bold;
    font-size: 16px;
    color: white;
    text-align: center;
}
</style>

<script>
function atualizarDados() {
    fetch("/data")
    .then(response => response.json())
    .then(data => {
        const valor_gases_bruto = data.fumaca;
        const gases_percentual = Math.min(100, Math.round((valor_gases_bruto / 4095) * 100));
        
        const temperatura = data.temperatura;
        const umidade = data.umidade;

        document.getElementById("termometro-fill-gases").style.height = gases_percentual + "%";
        document.getElementById("valor_gases").innerText = valor_gases_bruto + " ppm"; 

        const temperatura_fill = Math.min(100, temperatura);
        document.getElementById("temperatura-fill").style.height = temperatura_fill + "%";
        document.getElementById("temperatura-value").innerText = temperatura.toFixed(1) + "°C";

        const umidade_fill = Math.min(100, umidade);
        document.getElementById("umidade-fill").style.height = umidade_fill + "%";
        document.getElementById("umidade-value").innerText = umidade.toFixed(1) + "%";

        const alerta = document.getElementById("alerta");
        if (data.fogo === true) {
            alerta.style.display = "block";
            alerta.innerHTML = "🔥 ALERTA DE FOGO!";
        } else {
            alerta.style.display = "none";
        }

        document.getElementById("deviceStatus").innerText = "Online";
        document.getElementById("deviceStatus").style.color = "green";
    })
    .catch(erro => {
        document.getElementById("deviceStatus").innerText = "Offline";
        document.getElementById("deviceStatus").style.color = "red";
    });
}

setInterval(atualizarDados, 2000);
</script>

</head>

<body>

    <h1 class="titulo">Safe-Home 🏠</h1>

    <div class="card-container">

        <div class="card">
            <h2>Status do dispositivo</h2>
            <h2 id="deviceStatus">Conectando...</h2> 
        </div>

        <div class="card">
            <h2>Relatório de Monitoramento</h2>

            <div class="termometro-lado">

                <div class="termometro-container">
                    <h3>Nível de Gases</h3>
                    <div class="termometro-bg">
                        <div class="termometro-fill" id="termometro-fill-gases"></div>
                    </div>
                    <div id="valor_gases">0%</div>                 </div>

                <div class="temperatura-container">
                    <h3>Temperatura</h3>
                    <div class="temperatura-bg">
                        <div class="temperatura-fill" id="temperatura-fill"></div>
                    </div>
                    <div id="temperatura-value">0°C</div>
                </div>

                <div class="umidade-container">
                    <h3>Umidade</h3>
                    <div class="umidade-bg">
                        <div class="umidade-fill" id="umidade-fill"></div>
                    </div>
                    <div id="umidade-value">0%</div>
                </div>

            </div>

            <div id="alerta"></div>
        </div>

        <div class="card">
            <h2>Localização Monitorada</h2>
            <h2>📍 Sala de Estar</h2>
        </div>

    </div>

</body>
</html>
)rawliteral";

void setup()
{
      Serial.begin(115200);

      sensor_dht.begin();
      pinMode(PINO_BUZZER, OUTPUT);

      WiFi.softAP("ESP-Servidor", "12345678");
      Serial.println("Acesse: http://192.168.4.1");

      servidor.on("/", []()
                  {
    servidor.send_P(200, "text/html", site_html);
  });

      servidor.on("/data", []()
                  {
    float temp_atual = sensor_dht.readTemperature();
    float umi_atual = sensor_dht.readHumidity();
    int gas_atual = analogRead(PINO_MQ2);

    bool tem_fogo = (gas_atual > 2000 && temp_atual > 60);

    digitalWrite(PINO_BUZZER, tem_fogo ? HIGH : LOW);

   
    String resposta_json = "{";
    resposta_json += "\"fumaca\":" + String(gas_atual) + ",";
    resposta_json += "\"temperatura\":" + String(temp_atual) + ",";
    resposta_json += "\"umidade\":" + String(umi_atual) + ",";
    resposta_json += "\"fogo\":" + String(tem_fogo ? "true" : "false");
    resposta_json += "}";

    servidor.send(200, "application/json", resposta_json);
  });

      servidor.begin();
}
void loop()
{
      servidor.handleClient();
}