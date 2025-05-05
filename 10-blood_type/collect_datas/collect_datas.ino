#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

// Variables for HR and SpO2 calculation
float beatsPerMinute;
int beatAvg;
float spO2;
long lastBeat = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("MAX30102 HR/SpO2/PI Demo");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found!");
    while (1);
  }

  // Configure sensor
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);  // Turn Red LED to low brightness
  particleSensor.setPulseAmplitudeIR(0x0A);   // Turn IR LED to low brightness
}

void loop() {
  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();



  // 1. Check for a heartbeat (HR detection)
  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);
    beatAvg = (beatAvg * 0.9) + (beatsPerMinute * 0.1);  // Smoothing
  }

  // 2. Calculate SpO2 (using Red/IR ratio)
  float ratio = (float)redValue / irValue;
  spO2 = 110.0 - (25.0 * ratio);  // Approximate formula
  spO2 = constrain(spO2, 0.0, 100.0);

  // 3. Calculate Perfusion Index (PI)
  float pi = calculatePerfusionIndex(irValue);
  
  if (irValue > 10000){
    // Print results
    if(20 < beatAvg < 200){
      Serial.print(millis());
      Serial.print(beatAvg); Serial.print(","); 
      Serial.print(spO2); Serial.print(","); 
      Serial.println(pi);
    }
  }
  delay(100);  // Small delay to avoid serial spam
}

// Helper function to detect a heartbeat
boolean checkForBeat(long irValue) {
  static long prevIrValue = 0;
  static boolean prevState = false;
  static unsigned long lastBeat = 0;

  boolean beatDetected = (irValue > prevIrValue + 20) && prevState && (millis() - lastBeat > 300);
  prevIrValue = irValue;
  prevState = (irValue > 5000);  // Threshold for "valid signal"
  
  if (beatDetected) {
    lastBeat = millis();
  }
  return beatDetected;
}

// Helper function to calculate Perfusion Index (PI)
float calculatePerfusionIndex(long irValue) {
  static long dc = 0;       // Baseline (DC component)
  static long ac = 0;       // Peak-to-peak (AC component)
  static long minVal = 0;   // Min IR value in window
  static long maxVal = 0;   // Max IR value in window
  static unsigned long lastTime = 0;

  // Reset min/max every 2 seconds
  if (millis() - lastTime > 2000) {
    ac = maxVal - minVal;    // AC = peak-to-peak amplitude
    dc = (maxVal + minVal) / 2;  // DC = baseline
    minVal = irValue;
    maxVal = irValue;
    lastTime = millis();
  } else {
    if (irValue < minVal) minVal = irValue;
    if (irValue > maxVal) maxVal = irValue;
  }

  // Avoid division by zero
  if (dc == 0) return 0.0;

  // PI = (AC / DC) * 100%
  float pi = (ac * 100.0) / dc;
  return pi;
}