#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;
#define BUFFER_SIZE 100
#define FINGER_THRESHOLD 30000

uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];
int32_t spo2, heartRate;
int8_t validSPO2, validHeartRate;

// Moving average filter for HR
const int HR_AVG_SIZE = 5;
int hrBuffer[HR_AVG_SIZE] = {0};
int hrIndex = 0;

void setup() {
  Serial.begin(115200);
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found!");
    while (1);
  }

  // Optimized settings
  particleSensor.setup(60, 8, 2, 400, 411, 4096);
}

void loop() {
  if (particleSensor.getIR() < FINGER_THRESHOLD) {
    Serial.println("Place finger on sensor");
    delay(500);
    return;
  }

  // Fill buffers
  for (int i = 0; i < BUFFER_SIZE; i++) {
    while (!particleSensor.available()) particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  // Validate and smooth HR
  if (validHeartRate) {
    hrBuffer[hrIndex] = heartRate;
    hrIndex = (hrIndex + 1) % HR_AVG_SIZE;
    
    int avgHR = 0;
    for (int i = 0; i < HR_AVG_SIZE; i++) avgHR += hrBuffer[i];
    avgHR /= HR_AVG_SIZE;

    Serial.print("HR=");
    Serial.print(avgHR);
    Serial.print(", SpO2=");
    Serial.println(spo2);
  } else {
    Serial.println("Invalid reading. Hold steady.");
  }
  delay(1000);
}