#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define BUFFER_SIZE 100  // Increased buffer size for better accuracy
#define FINGER_THRESHOLD 30000  // Adjust this based on your testing

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin not used
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create your custom WiFi network
const char* ssid = "MyHealth";
const char* password = "123D56HI";  // Must be at least 8 characters

// Create a web server on port 80
ESP8266WebServer server(80);
MAX30105 particleSensor;

uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t spo2, heartRate;
int8_t validSPO2, validHeartRate;
float temperature;
int avgHR;

// Moving average filter for HR
const int HR_AVG_SIZE = 5;
int hrBuffer[HR_AVG_SIZE] = {0};
int hrIndex = 0;

// Initialize OLED
void setupOLED() {
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // I2C address 0x3C
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

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  setupOLED();

  // Set ESP8266 as Access Point
  WiFi.softAP(ssid, password);

  Serial.println("Access Point Started");
  Serial.print("IP address: ");
  IPAddress myIP = WiFi.softAPIP(); // Typically 192.168.4.1

  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 24);
  oled.println(myIP);
  oled.display();


  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }

  byte ledBrightness = 60;    // Options: 0=Off to 255=50mA
  byte sampleAverage = 8;     // Increased averaging for stability
  byte ledMode = 2;           // Red + IR mode
  byte sampleRate = 400;      // Higher sample rate
  int pulseWidth = 411;       // Options: 69, 118, 215, 411
  int adcRange = 4096;        // Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  
  // Enable AC filtering
  particleSensor.enableDIETEMPRDY();

   // Define web server routes
  server.on("/", handleRoot);       // Main page
  server.on("/data", handleData);  // JSON API for sensor data
  server.begin();
  Serial.println("HTTP server started");
}

void updateOLED() {
  oled.clearDisplay();
  oled.setCursor(0, 0);

  // Display HR
  oled.print("HR: ");
  oled.print(avgHR);
  oled.print(" bpm");

  // Display SpO2 (next line)
  oled.setCursor(0, 12);
  oled.print("SpO2: ");
  oled.print(spo2);
  oled.print("%");

  // Display Temperature (next line)
  oled.setCursor(0, 24);
  oled.print("Temp: ");
  oled.print(temperature);
  oled.print("C");

  oled.display();
}

void loop() {
  server.handleClient();
  int32_t irValue = particleSensor.getIR();
  
  if (irValue < FINGER_THRESHOLD) {
    Serial.println("No finger detected. Place your finger on the sensor.");
    delay(500);
    return;
  }

  // Fill buffers
  for (int i = 0; i < BUFFER_SIZE; i++) {
    while (!particleSensor.available()) {
      particleSensor.check();
    }
    
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
    
    // Small delay to allow for sensor settling
    delay(10);
  }

  // Calculate heart rate and SpO2
  maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  // Validate and smooth HR
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
    Serial.println("Invalid reading. Hold steady.");
  }
  temperature = particleSensor.readTemperature();  // Returns temperature in °C
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");
  updateOLED();

  delay(1000);  // Longer delay between readings
}

void handleData() {
  String json = "{";
  json += "\"hr\":" + String(avgHR) + ",";
  json += "\"spo2\":" + String(spo2) + ",";
  json += "\"temp\":" + String(temperature);
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
  <div class="sensor">Temperature: <span id="temp">--</span> °C</div>
</body>
</html>
)=====";
  server.send(200, "text/html", html);
}