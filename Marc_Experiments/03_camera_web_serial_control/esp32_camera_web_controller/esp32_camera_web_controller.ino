/**********************************************************************
  Filename    : esp32_camera_web_controller.ino
  Project     : Marc_Experiments/03_camera_web_serial_control
  Description : Camera web server + simple rover command web page.

  This experiment reuses the known-good Freenove camera web server files
  from Sketches/07.1_Camera_Test without modifying the original sketch.

  Camera page:
    http://<ESP32-IP>/

  Rover control page:
    http://<ESP32-IP>:82/

  Rover command endpoint:
    http://<ESP32-IP>:82/cmd?move=F
    http://<ESP32-IP>:82/cmd?move=B
    http://<ESP32-IP>:82/cmd?move=L
    http://<ESP32-IP>:82/cmd?move=R
    http://<ESP32-IP>:82/cmd?move=S

  IMPORTANT PIN NOTE:
    GPIO5 worked for the earlier serial-only test, but the WROVER camera
    config uses GPIO5 as camera Y3. Do NOT use GPIO5 while the camera is active.

    Default rover UART TX below is GPIO15. If GPIO15 does not work on your
    board, try another exposed free GPIO that is not used by the camera.
**********************************************************************/

#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>

// Reuse the existing, known-good camera config and Wi-Fi credentials.
// This keeps the original 07.1_Camera_Test sketch untouched.
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

// Camera server uses port 80/stream handling from the original Freenove code.
// This lightweight rover controller runs separately on port 82.
WebServer roverServer(82);
HardwareSerial UnoSerial(1);

// GPIO5 conflicts with the WROVER camera pin map.
// GPIO15 is the current test candidate for ESP -> UNO UART TX while camera is active.
#define ROVER_UART_TX_PIN 15
#define ROVER_UART_BAUD   9600

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("ESP32 camera + rover web controller starting...");

  // Start UART to UNO.
  // RX unused for now (-1), TX = ROVER_UART_TX_PIN.
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
  s->set_vflip(s, 0);        // 1-Upside down, 0-No operation
  s->set_hmirror(s, 0);      // 1-Reverse left and right, 0-No operation
  s->set_brightness(s, 1);   // Up the brightness just a bit
  s->set_saturation(s, -1);  // Lower the saturation

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

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  Serial.print("Rover Control Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println(":82' to connect");
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
  Serial.println("Rover control server started on port 82");
}

void handleRoverControlPage() {
  String html = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Rover Control</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin: 20px; }
    .grid { display: inline-grid; grid-template-columns: 90px 90px 90px; gap: 10px; }
    button { font-size: 22px; padding: 18px; border-radius: 10px; }
    .stop { font-weight: bold; }
    #status { margin-top: 16px; font-size: 16px; }
    a { display: block; margin-top: 20px; }
  </style>
</head>
<body>
  <h2>ESP32 Rover Control</h2>
  <div class="grid">
    <div></div><button onclick="sendCmd('F')">F</button><div></div>
    <button onclick="sendCmd('L')">L</button><button class="stop" onclick="sendCmd('S')">S</button><button onclick="sendCmd('R')">R</button>
    <div></div><button onclick="sendCmd('B')">B</button><div></div>
  </div>
  <div id="status">Ready</div>
  <a href="/" onclick="location.href='http://' + location.hostname + '/'; return false;">Open Camera Page</a>

<script>
async function sendCmd(cmd) {
  const status = document.getElementById('status');
  status.textContent = 'Sending ' + cmd + '...';
  try {
    const res = await fetch('/cmd?move=' + encodeURIComponent(cmd));
    const text = await res.text();
    status.textContent = text;
  } catch (e) {
    status.textContent = 'Error: ' + e;
  }
}
</script>
</body>
</html>
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

// Pull in the original Freenove camera web server implementation.
// This lets this experiment reuse the known-good camera page without copying
// or modifying the original camera server files.
#include "../../../Sketches/07.1_Camera_Test/app_httpd.cpp"
