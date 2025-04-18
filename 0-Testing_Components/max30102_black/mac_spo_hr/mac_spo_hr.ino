#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define BUFFER_SIZE 100  // Increased buffer size for better accuracy
#define FINGER_THRESHOLD 30000  // Adjust this based on your testing

// Create your custom WiFi network
const char* ssid = "MyHealth";
const char* password = "123D56HI";  // Must be at least 8 characters

// Create a web server on port 80
ESP8266WebServer server(80);
MAX30105 particleSensor;

uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Set ESP8266 as Access Point
  WiFi.softAP(ssid, password);

  Serial.println("Access Point Started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP()); // Typically 192.168.4.1

  // Define a simple webpage
  server.on("/", []() {
    server.send(200, "text/html", "<h1>Hello from ESP8266 AP!</h1>");
  });

  server.begin();
  Serial.println("Web server started");

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

  // Only display valid readings
  if (validHeartRate && validSPO2) {
    Serial.print("HR=");
    Serial.print(heartRate);
    Serial.print("bpm, SPO2=");
    Serial.print(spo2);
    Serial.println("%");
  } else {
    Serial.println("Invalid reading. Please keep your finger steady.");
  }
  float temperature = particleSensor.readTemperature();  // Returns temperature in °C
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");

  delay(1000);  // Longer delay between readings
}