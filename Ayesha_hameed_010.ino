#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 48
#define NUM_LEDS 1

Adafruit_NeoPixel led(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "RGB-TEST";
const char* password = "12345678";

WebServer server(80);
bool robotRunning = false;

void setRobotState(bool running) {
  robotRunning = running;

  if (running) {
    led.setPixelColor(0, led.Color(0, 255, 0));
    Serial.println("ROBOT OPERATIONAL - GREEN LED");
  } else {
    led.setPixelColor(0, led.Color(255, 0, 0));
    Serial.println("SAFETY HALT - RED LED");
  }

  led.show();
}

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Ayesha | Robot Control</title>

<style>
* {
  box-sizing: border-box;
}

body {
  margin: 0;
  min-height: 100vh;
  background: linear-gradient(135deg, #210b46, #602080, #ba438e);
  color: white;
  font-family: Arial, sans-serif;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 20px;
}

.card {
  width: 100%;
  max-width: 460px;
  padding: 35px 25px;
  text-align: center;
  background: rgba(255,255,255,0.12);
  border: 1px solid rgba(255,255,255,0.25);
  border-radius: 24px;
  box-shadow: 0 15px 45px rgba(0,0,0,0.3);
}

h1 {
  font-size: 29px;
  margin-bottom: 10px;
}

.subtitle {
  color: #f5d0fe;
  font-size: 15px;
}

.student {
  margin-top: 25px;
  padding: 16px;
  background: rgba(255,255,255,0.13);
  border-radius: 14px;
  font-size: 21px;
  font-weight: bold;
  color: #fbcfe8;
  letter-spacing: 1px;
}

.status-box {
  margin: 25px 0;
  padding: 22px 10px;
  background: rgba(0,0,0,0.2);
  border-radius: 16px;
}

.status-label {
  font-size: 13px;
  color: #e9d5ff;
  margin-bottom: 12px;
}

.status {
  font-size: 19px;
  font-weight: bold;
}

.green {
  color: #86efac;
}

.red {
  color: #fda4af;
}

button {
  display: block;
  width: 100%;
  padding: 18px;
  margin: 14px 0;
  border: none;
  border-radius: 12px;
  color: white;
  font-size: 17px;
  font-weight: bold;
  cursor: pointer;
  transition: 0.2s;
}

.start {
  background: linear-gradient(90deg, #059669, #34d399);
  box-shadow: 0 5px 20px #05966955;
}

.stop {
  background: linear-gradient(90deg, #e11d48, #fb7185);
  box-shadow: 0 5px 20px #e11d4855;
}

button:active {
  transform: scale(0.97);
}

#message {
  color: #f5d0fe;
  font-size: 14px;
  min-height: 20px;
}

.footer {
  margin-top: 25px;
  padding-top: 18px;
  border-top: 1px solid #ffffff33;
  color: #e9d5ff;
  font-size: 12px;
}
</style>
</head>

<body>
<div class="card">

<h1>ROBOT CONTROL</h1>

<p class="subtitle">
ESP32 Wireless Control System
</p>

<div class="student">
AYESHA
</div>

<div class="status-box">
  <div class="status-label">CURRENT ROBOT STATUS</div>
  <div id="status" class="status">Loading...</div>
</div>

<button class="start" onclick="control(1)">
START ROBOT
</button>

<button class="stop" onclick="control(0)">
SAFETY HALT
</button>

<p id="message"></p>

<div class="footer">
ESP32-S3 | Wireless Dashboard | Port 80
</div>

</div>

<script>
async function refreshStatus() {
  try {
    const response = await fetch('/robot/status');

    if (!response.ok) {
      throw new Error('Server Error');
    }

    const data = await response.json();
    const status = document.getElementById('status');

    if (data.running) {
      status.textContent = 'OPERATIONAL - GREEN LED';
      status.className = 'status green';
    } else {
      status.textContent = 'SAFETY HALT - RED LED';
      status.className = 'status red';
    }

  } catch (error) {
    document.getElementById('message').textContent =
      'Network Error';
  }
}

async function control(state) {
  try {
    const response = await fetch(
      '/robot/status?state=' + state
    );

    if (!response.ok) {
      throw new Error('Command Failed');
    }

    document.getElementById('message').textContent =
      'Command Successful';

    await refreshStatus();

  } catch (error) {
    document.getElementById('message').textContent =
      'Network Error';
  }
}

refreshStatus();
setInterval(refreshStatus, 2000);
</script>

</body>
</html>
)rawliteral";

void handleRobotStatus() {
  if (server.hasArg("state")) {
    String state = server.arg("state");

    if (state == "1") {
      setRobotState(true);
    } else if (state == "0") {
      setRobotState(false);
    } else {
      server.send(
        400,
        "application/json",
        "{\"error\":\"Invalid state\"}"
      );
      return;
    }
  }

  String response = "{\"running\":";
  response += robotRunning ? "true" : "false";
  response += "}";

  server.send(200, "application/json", response);
}

void setup() {
  Serial.begin(115200);

  led.begin();
  led.setBrightness(50);
  setRobotState(false);

  WiFi.mode(WIFI_AP);

  bool started = WiFi.softAP(
    ssid,
    password,
    1,
    0,
    4
  );

  if (!started) {
    Serial.println("WiFi Access Point Failed");
    return;
  }

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32 ROBOT CONTROL");
  Serial.println("Name: Ayesha");
  Serial.println("================================");

  Serial.println("WiFi Access Point Started");

  Serial.print("Network Name: ");
  Serial.println(ssid);

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", webpage);
  });

  server.on(
    "/robot/status",
    HTTP_GET,
    handleRobotStatus
  );

  server.onNotFound([]() {
    server.send(
      404,
      "text/plain",
      "Page Not Found"
    );
  });

  server.begin();

  Serial.println("HTTP SERVER STARTED ON PORT 80");
  Serial.println("Open: http://192.168.4.1");
}

void loop() {
  server.handleClient();
}
