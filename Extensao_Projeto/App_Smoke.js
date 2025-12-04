let conectado = false;
let tempAtual = 29.5;
let umiAtual = 50.0;
let gasAtual = 0;

const REDE_ESP_SERVIDOR = "ESP-SERVIDOR";

// --- GERA VALORES ALEATÓRIOS ---
function gerarDadosFake() {

    tempAtual += (Math.random() * 0.6 - 0.3); 
    umiAtual  += (Math.random() * 1 - 0.5);
    gasAtual  = Math.floor(Math.random() * 100);

    tempAtual = Math.min(80, Math.max(20, tempAtual));
    umiAtual  = Math.min(90, Math.max(20, umiAtual));

    return {
        fumaca: gasAtual,
        temperatura: tempAtual.toFixed(1),
        umidade: umiAtual.toFixed(1),
    };
}

// --- ATUALIZA TERMÔMETROS & ALERTAS ---
function atualizarSimulacao() {
    if (!conectado) return;

    const data = gerarDadosFake();

    // GASES
    document.getElementById("termometro-fill-gases").style.height = data.fumaca + "%";
    document.getElementById("termometro-value-gases").innerText = data.fumaca + "%";

    // TEMPERATURA
    const temp = Number(data.temperatura);
    const temp_percent = Math.round((temp / 80) * 100);
    document.getElementById("temperatura-fill").style.height = temp_percent + "%";
    document.getElementById("temperatura-value").innerText = temp.toFixed(1) + "°C";

    // UMIDADE
    const umidade = Number(data.umidade);
    document.getElementById("umidade-fill").style.height = umidade + "%";
    document.getElementById("umidade-value").innerText = umidade + "%";

    atualizarStatusPerigo(temp, umidade, gasAtual);
}

// --- SISTEMA DE ALERTAS ---
function atualizarStatusPerigo(temp, umi, gas) {
    const alerta = document.getElementById("alerta");

    alerta.style.display = "none";

    if (gas > 60 || temp > 60) {
        alerta.style.display = "block";
        alerta.style.background = "#dc2626";
        alerta.innerText = "⚠ PERIGO! NÍVEIS CRÍTICOS DETECTADOS!";
    }
    else if (gas > 30 || temp > 40) {
        alerta.style.display = "block";
        alerta.style.background = "#fbbf24";
        alerta.innerText = "⚠ ALERTA! Níveis acima do normal.";
    }
    else {
        alerta.style.display = "none";
    }
}

// --- PERGUNTA SE ESTÁ NA REDE ESP ---
function perguntarRede() {
    const salvo = localStorage.getItem("wifi_esp_conectado");

    if (salvo === "sim" || salvo === "nao") return;

    const confirma = confirm(`Para monitorar o ESP32, você precisa estar na rede '${REDE_ESP_SERVIDOR}'.\n\nEstá conectado nela agora?`);
    localStorage.setItem("wifi_esp_conectado", confirma ? "sim" : "nao");
}

// --- CONTROLE DE CONEXÃO ---
function verificarConexao() {
    const statusEl = document.getElementById("deviceStatus");
    const conectadoNaRedeESP = localStorage.getItem("wifi_esp_conectado") === "sim";

    statusEl.innerText = conectadoNaRedeESP
        ? "Conectado à Rede ESP"
        : "Offline (Não na rede ESP)";

    statusEl.style.color = conectadoNaRedeESP ? "green" : "red";

    conectado = conectadoNaRedeESP;
}

// --- INICIALIZAÇÃO ---
perguntarRede();
verificarConexao();

setInterval(() => {
    verificarConexao();
    atualizarSimulacao();
}, 1500);

