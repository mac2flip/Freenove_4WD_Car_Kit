/**********************************************************************
  Filename    : esp32_camera_web_controller.ino
  Project     : Marc_Experiments/03_camera_web_serial_control
  Description : Camera web server + hold-to-drive rover command page.

  Camera page:
    http://<ESP32-IP>/

  Combined rover page:
    http://<ESP32-IP>:82/
**********************************************************************/

#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>

#include "../../../Sketches/07.1_Camera_Test/board_config.h"
#include "../../../Sketches/07.1_Camera_Test/credentials.h"

const char *ssid_Router     = WIFI_SSID;
const char *password_Router = WIFI_PASSWORD;

camera_config_t config;

void startCameraServer();
void camera_init();
void startRoverControlServer();
void handleRoverControlPage();
void handleRoverCommand();
void sendRoverCommand(char cmd);

WebServer roverServer(82);
HardwareSerial UnoSerial(1);

#define ROVER_UART_TX_PIN 15
#define ROVER_UART_BAUD   9600

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("ESP32 camera rover page starting...");

  UnoSerial.begin(ROVER_UART_BAUD, SERIAL_8N1, -1, ROVER_UART_TX_PIN);
  Serial.print("UNO UART TX pin: GPIO");
  Serial.println(ROVER_UART_TX_PIN);

  camera_init();

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 0);
  s->set_hmirror(s, 0);
  s->set_brightness(s, 1);
  s->set_saturation(s, -1);

  WiFi.begin(ssid_Router, password_Router);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();
  startRoverControlServer();

  Serial.print("Camera page: http://");
  Serial.println(WiFi.localIP());

  Serial.print("Combined page: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":82");
}

void loop() {
  roverServer.handleClient();
}

void camera_init() {
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 2;
}

void startRoverControlServer() {
  roverServer.on("/", HTTP_GET, handleRoverControlPage);
  roverServer.on("/cmd", HTTP_GET, handleRoverCommand);
  roverServer.begin();
  Serial.println("Rover web page started on port 82");
}

void handleRoverControlPage() {
  String html = R"rawliteral(
<!doctype html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<title>Rover Camera Page</title>
<style>
body{font-family:Arial,sans-serif;text-align:center;margin:0;padding:12px;background:#111;color:#eee}.layout{display:flex;flex-direction:column;gap:12px;align-items:center;max-width:900px;margin:0 auto}.video-wrap{width:100%;max-width:640px;background:#000;border:1px solid #333;border-radius:10px;overflow:hidden}#stream{display:block;width:100%;height:auto}.controls{width:100%;max-width:340px;padding:12px;background:#1c1c1c;border:1px solid #333;border-radius:12px}.grid{display:grid;grid-template-columns:1fr 1fr 1fr;gap:10px}button{font-size:24px;padding:18px 10px;border-radius:12px;border:1px solid #555;background:#2b2b2b;color:#fff;touch-action:none;user-select:none}button:active,.active{transform:scale(.97);background:#3b3b3b}.stop{font-weight:bold;background:#4a1f1f}#status{margin-top:12px;min-height:22px;font-size:16px;color:#cfcfcf}.hint{font-size:13px;color:#aaa}a{color:#8ab4ff;margin:0 8px}@media(min-width:800px){.layout{flex-direction:row;align-items:flex-start;justify-content:center}.controls{max-width:300px}}
</style></head><body>
<h2>ESP32 Rover Camera Page</h2>
<div class="layout"><div class="video-wrap"><img id="stream" alt="Camera stream"></div>
<div class="controls"><div class="grid">
<div></div><button data-cmd="F">F</button><div></div>
<button data-cmd="L">L</button><button class="stop" onclick="tapStop()">S</button><button data-cmd="R">R</button>
<div></div><button data-cmd="B">B</button><div></div>
</div><div id="status">Ready</div><div class="hint">Hold F/B/L/R to move. Release to stop. S is emergency stop.</div>
<p><a href="#" onclick="openCameraPage();return false;">Camera settings</a> <a href="#" onclick="reloadStream();return false;">Reload stream</a></p>
</div></div>
<script>
let holdTimer=null, activeCmd=null;
function streamUrl(){return 'http://'+location.hostname+':81/stream'}
function cameraPageUrl(){return 'http://'+location.hostname+'/'}
function reloadStream(){document.getElementById('stream').src=streamUrl()+'?t='+Date.now()}
function openCameraPage(){location.href=cameraPageUrl()}
async function sendCmd(cmd){const s=document.getElementById('status');try{const r=await fetch('/cmd?move='+encodeURIComponent(cmd));s.textContent=await r.text()}catch(e){s.textContent='Error: '+e}}
function startHold(cmd,btn){if(activeCmd)return;activeCmd=cmd;btn.classList.add('active');sendCmd(cmd);holdTimer=setInterval(()=>sendCmd(cmd),200)}
function stopHold(){if(holdTimer){clearInterval(holdTimer);holdTimer=null}document.querySelectorAll('button').forEach(b=>b.classList.remove('active'));if(activeCmd){activeCmd=null;sendCmd('S')}}
function tapStop(){stopHold();sendCmd('S')}
document.querySelectorAll('button[data-cmd]').forEach(btn=>{const c=btn.dataset.cmd;btn.addEventListener('mousedown',e=>{e.preventDefault();startHold(c,btn)});btn.addEventListener('touchstart',e=>{e.preventDefault();startHold(c,btn)},{passive:false});});
document.addEventListener('mouseup',stopHold);document.addEventListener('touchend',stopHold);document.addEventListener('touchcancel',stopHold);window.addEventListener('blur',stopHold);
reloadStream();
</script></body></html>
)rawliteral";

  roverServer.send(200, "text/html", html);
}

void handleRoverCommand() {
  if (!roverServer.hasArg("move")) {
    roverServer.send(400, "text/plain", "Missing move argument. Use /cmd?move=F");
    return;
  }

  String move = roverServer.arg("move");
  if (move.length() == 0) {
    roverServer.send(400, "text/plain", "Empty move argument");
    return;
  }

  char cmd = toupper(move.charAt(0));
  if (cmd != 'F' && cmd != 'B' && cmd != 'L' && cmd != 'R' && cmd != 'S') {
    roverServer.send(400, "text/plain", "Invalid command. Use F, B, L, R, or S.");
    return;
  }

  sendRoverCommand(cmd);
  roverServer.send(200, "text/plain", String("Sent command: ") + cmd);
}

void sendRoverCommand(char cmd) {
  UnoSerial.write(cmd);
  Serial.print("Sent rover command: ");
  Serial.println(cmd);
}

#include "../../../Sketches/07.1_Camera_Test/app_httpd.cpp"
