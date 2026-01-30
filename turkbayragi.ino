#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Wi-Fi bilgileri
const char* ssid = "SUPERONLINE_Wi-Fi_4KCX";
const char* password = "N4sGXUYG753R";

// LED pinleri
const int led1Pin = 2;
const int led2Pin = 4;

// PWM ayarları
const int freq = 5000;
const int ledChannel1 = 0;
const int ledChannel2 = 1;
const int resolution = 8; // 0-255

// Web sunucu
AsyncWebServer server(80);

// LED durumu
int brightness1 = 0;
int brightness2 = 0;
int effect = 0;

// **Web sayfasını burada global tanımla**
const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
<meta charset="UTF-8">
<title>ESP32 Control Core</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<style>
:root{
  --bg:#020617;
  --card:#0f172aee;
  --neon:#22d3ee;
  --accent:#38bdf8;
}

*{box-sizing:border-box}

body{
  margin:0;
  min-height:100vh;
  background:
    radial-gradient(circle at top, #0ea5e933, transparent 40%),
    linear-gradient(180deg,#020617,#000);
  font-family: system-ui, sans-serif;
  color:white;
  display:flex;
  justify-content:center;
  align-items:center;
}

.card{
  width:95%;
  max-width:420px;
  background:var(--card);
  border-radius:22px;
  padding:25px;
  backdrop-filter: blur(15px);
  border:1px solid #22d3ee44;
  box-shadow:
    0 0 40px #22d3ee33,
    inset 0 0 25px #22d3ee22;
}

h1{
  text-align:center;
  font-size:22px;
  margin-bottom:25px;
  color:var(--neon);
  letter-spacing:1px;
}

.group{margin-bottom:22px}

.group span{
  font-size:12px;
  opacity:.7;
}

.range-wrap{
  display:flex;
  align-items:center;
  gap:12px;
}

.value{
  width:40px;
  text-align:right;
  color:var(--accent);
  font-weight:bold;
}

input[type=range]{
  flex:1;
  -webkit-appearance:none;
  height:6px;
  border-radius:6px;
  background:linear-gradient(90deg,var(--neon),#1e293b);
  outline:none;
}

input[type=range]::-webkit-slider-thumb{
  -webkit-appearance:none;
  width:18px;
  height:18px;
  border-radius:50%;
  background:var(--neon);
  box-shadow:0 0 10px var(--neon);
  cursor:pointer;
}

select{
  width:100%;
  padding:12px;
  border-radius:10px;
  border:1px solid #22d3ee55;
  background:#020617;
  color:white;
}

button{
  width:100%;
  padding:14px;
  border-radius:14px;
  border:none;
  font-size:16px;
  font-weight:bold;
  background:linear-gradient(90deg,var(--neon),var(--accent));
  color:#020617;
  cursor:pointer;
  box-shadow:0 0 20px #22d3ee99;
  transition:.2s;
}

button:hover{
  transform:scale(1.05);
  box-shadow:0 0 35px #22d3ee;
}
</style>
</head>

<body>
<div class="card">
  <h1>ESP32 CONTROL CORE</h1>

  <div class="group">
    <span>LED 1</span>
    <div class="range-wrap">
      <input type="range" id="led1" min="0" max="255" value="0" oninput="v1.textContent=this.value">
      <div class="value" id="v1">0</div>
    </div>
  </div>

  <div class="group">
    <span>LED 2</span>
    <div class="range-wrap">
      <input type="range" id="led2" min="0" max="255" value="0" oninput="v2.textContent=this.value">
      <div class="value" id="v2">0</div>
    </div>
  </div>

  <div class="group">
    <span>Efekt Modu</span>
    <select id="effect">
      <option value="0">Kapalı</option>
      <option value="1">Sabit Açık</option>
      <option value="2">Nefes Alma</option>
      <option value="3">Hızlı Blink</option>
      <option value="4">Yavaş Blink</option>
    </select>
  </div>

  <button onclick="updateLED()">UYGULA</button>
</div>

<script>
function updateLED(){
  let l1 = led1.value;
  let l2 = led2.value;
  let e  = effect.value;
  fetch(`/set?led1=${l1}&led2=${l2}&effect=${e}`);
}
</script>
</body>
</html>

)rawliteral";

void setup() {
  Serial.begin(115200);

  // PWM setup
  ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel2, freq, resolution);
  ledcAttachPin(led1Pin, ledChannel1);
  ledcAttachPin(led2Pin, ledChannel2);

  // Wi-Fi bağlan
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  // Web sayfa
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);  // send_P yerine send kullandık
  });

  // LED kontrolu
  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request){
    if(request->hasParam("led1")) brightness1 = request->getParam("led1")->value().toInt();
    if(request->hasParam("led2")) brightness2 = request->getParam("led2")->value().toInt();
    if(request->hasParam("effect")) effect = request->getParam("effect")->value().toInt();
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  applyEffect();
}

// LED efektleri
void applyEffect() {
  switch(effect){
    case 0: // Kapalı
      ledcWrite(ledChannel1, 0);
      ledcWrite(ledChannel2, 0);
      break;
    case 1: // Açık sabit
      ledcWrite(ledChannel1, brightness1);
      ledcWrite(ledChannel2, brightness2);
      break;
    case 2: // Nefes alma
      for(int i=0;i<=brightness1;i++){
        ledcWrite(ledChannel1,i);
        ledcWrite(ledChannel2,i);
        delay(5);
      }
      for(int i=brightness1;i>=0;i--){
        ledcWrite(ledChannel1,i);
        ledcWrite(ledChannel2,i);
        delay(5);
      }
      break;
    case 3: // Yanıp sönme hızlı
      ledcWrite(ledChannel1, brightness1);
      ledcWrite(ledChannel2, brightness2);
      delay(100);
      ledcWrite(ledChannel1, 0);
      ledcWrite(ledChannel2, 0);
      delay(100);
      break;
    case 4: // Yanıp sönme yavaş
      ledcWrite(ledChannel1, brightness1);
      ledcWrite(ledChannel2, brightness2);
      delay(500);
      ledcWrite(ledChannel1, 0);
      ledcWrite(ledChannel2, 0);
      delay(500);
      break;
  }
}
