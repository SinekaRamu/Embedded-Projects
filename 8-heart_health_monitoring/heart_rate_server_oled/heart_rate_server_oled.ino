#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define BUFFER_SIZE 100
#define FINGER_THRESHOLD 30000

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "MyHealth";
const char* password = "123D56HI";

ESP8266WebServer server(80);
MAX30105 particleSensor;

IPAddress myIP;
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t spo2, heartRate;
int8_t validSPO2, validHeartRate;
float temperature;
int avgHR, avgSPO;

const int HR_AVG_SIZE = 5;
int hrBuffer[HR_AVG_SIZE] = {0};
int hrIndex = 0;

void setupOLED() {
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed!");
    while (1);
  }
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 0);
  oled.println("Initializing...");
  oled.display();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  setupOLED();
  delay(3000);

  WiFi.softAP(ssid, password);
  myIP = WiFi.softAPIP();
  Serial.println("Access Point Started");
  Serial.print("IP address: ");
  Serial.println(myIP);

  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("WiFi AP Started");
  oled.setCursor(0, 24);
  oled.println(myIP);
  oled.display();
  delay(3000);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }

  particleSensor.setup(60, 8, 2, 400, 411, 4096);
  particleSensor.enableDIETEMPRDY();

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP server started");
}

void updateOLED() {
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.print("HR: ");
  oled.print(avgHR);
  oled.print(" bpm");

  oled.setCursor(0, 12);
  oled.print("SpO2: ");
  if (validSPO2) {
    oled.print(avgSPO);
    oled.print("%");
  } else {
    oled.print("--%");
  }

  oled.setCursor(0, 24);
  oled.print("Temp: ");
  oled.print(String(temperature, 1));
  oled.print("C");

  oled.display();
}

void loop() {
  server.handleClient();

  int32_t irValue = particleSensor.getIR();
  if (irValue < FINGER_THRESHOLD) {
    Serial.println("No finger detected.");
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.print("NO FINGER DETECTED");
    oled.setCursor(0, 24);
    oled.println(myIP);
    oled.display();
    delay(500);
    return;
  }

  for (int i = 0; i < BUFFER_SIZE; i++) {
    while (!particleSensor.available()) {
      particleSensor.check();
    }
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
    delay(10);
  }

  maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer,
                                         &spo2, &validSPO2, &heartRate, &validHeartRate);

  if(validSPO2){
    avgSPO = spo2;
  }
  if (validHeartRate) {
    hrBuffer[hrIndex] = heartRate;
    hrIndex = (hrIndex + 1) % HR_AVG_SIZE;
    avgHR = 0;
    for (int i = 0; i < HR_AVG_SIZE; i++) avgHR += hrBuffer[i];
    avgHR /= HR_AVG_SIZE;
    Serial.print("HR=");
    Serial.print(avgHR);
    Serial.print(", SpO2=");
    Serial.println(spo2);
  } else {
    Serial.println("Invalid HR reading.");
  }

  temperature = particleSensor.readTemperature();
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.println(" C");

  updateOLED();
  delay(1000);
}

void handleData() {
  String json = "{";
  json += "\"hr\":" + String(avgHR) + ",";
  json += "\"spo2\":" + String(avgSPO) + ",";
  json += "\"temp\":" + String(temperature, 1);
  json += "}";
  server.send(200, "application/json", json);
}

void handleRoot() {
  String html = R"=====(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP8266 Health Monitor</title>
    <style>
      body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }
      .sensor { font-size: 24px; margin: 20px; }
      #hr, #spo2, #temp { font-weight: bold; color: #0066cc; }
    </style>
    <script>
      function updateData() {
        fetch("/data")
          .then(response => response.json())
          .then(data => {
            document.getElementById("hr").textContent = data.hr;
            document.getElementById("spo2").textContent = data.spo2;
            document.getElementById("temp").textContent = data.temp;
            setTimeout(updateData, 10000); // Refresh every 2 sec
          });
      }
      window.onload = updateData;
    </script>
  </head>
  <body>
    <h1>ESP8266 Health Monitor</h1>
    <div class="sensor">Heart Rate: <span id="hr">--</span> bpm</div>
    <div class="sensor">SpO2: <span id="spo2">--</span> %</div>
    <div class="sensor">Temperature: <span id="temp">--</span> &#8451;</div>
  </body>
  </html>
  )=====";
  server.send(200, "text/html", html);
}