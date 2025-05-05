#include <Wire.h>
#include "MAX3 0105.h"
#include "heartRate.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_NeoPixel.h>

MAX30105 particleSensor;

#define SCALING 12
#define TRACE_SPEED 0.5
#define TRACE_MIDDLE_Y_POSITION 90
#define TRACE_HEIGHT 60
#define HALF_TRACE_HEIGHT TRACE_HEIGHT / 2
#define TRACE_MIN_Y TRACE_MIDDLE_Y_POSITION - HALF_TRACE_HEIGHT + 1
#define TRACE_MAX_Y TRACE_MIDDLE_Y_POSITION + HALF_TRACE_HEIGHT - 1

#define TFT_CS   D8
#define TFT_RST  D0
#define TFT_DC   D4
#define TFT_SDA  D7
#define TFT_SCL  D5

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define NEOPIXEL_PIN D6
#define NUMPIXELS 16
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

long lastBeat = 0;
float beatsPerMinute = 0.0;
float spO2 = 0.0;

bool isInhaling = false;
bool isExhaling = false;
int stableCount = 0;
const int inhaleExhaleThreshold = 5;
long irMovingAverage = 0;
const int smoothingFactor = 20;

const long MIN_IR_VALUE = 5000;
bool fingerDetected = false;

// Variables for Neopixel Heartbeat Animation
uint8_t heartBrightness = 0;
bool increasingBrightness = true;
unsigned long lastHeartBeatUpdate = 0;
const unsigned long heartbeatInterval = 10;  // Faster interval for animation

void setup() {
  tft.initR(INITR_GREENTAB);   
  tft.setRotation(3); 
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST7735_WHITE);

  pixels.begin(); 
  pixels.clear();
  pixels.show();

  Serial.begin(115200);
  Serial.println("Initializing...");

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }

  byte ledBrightness = 0x1F;
  byte sampleAverage = 8;
  byte ledMode = 3;
  int sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  particleSensor.enableDIETEMPRDY(); 

  tft.setTextSize(2);
  tft.setCursor(0, 10);
  tft.print("BPM");
  tft.setTextSize(1);
  tft.setCursor(1, 0);
  tft.print("Heart Health Monitoring");
  tft.setTextSize(2);
  tft.drawRect(0, TRACE_MIN_Y - 1, 160, TRACE_HEIGHT + 2, ST7735_BLUE);  
}

void loop() {
  static float lastx = 1;
  static int lasty = TRACE_MIDDLE_Y_POSITION;
  static float x = 1;
  int32_t y;
  static uint32_t tsLastReport = 0;
  static int32_t SensorOffset = 10000;

  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();

  // Check if a finger is detected based on IR value
  fingerDetected = (irValue > MIN_IR_VALUE);

  if (fingerDetected) {
    // BPM detection
    if (checkForBeat(irValue)) {
      long delta = millis() - lastBeat;

      if (delta > 300) {
        lastBeat = millis();
        float bpm = 60 / (delta / 1000.0);

        if (bpm > 40 && bpm < 180) {
          beatsPerMinute = bpm;
          tft.fillRect(0, 30, 160, 20, ST7735_BLACK); 
          tft.setCursor(0, 30);

          if (beatsPerMinute < 60 || beatsPerMinute > 110)
            tft.setTextColor(ST7735_BLUE);
          else
            tft.setTextColor(ST7735_GREEN);
          tft.setTextSize(2);
          tft.print(beatsPerMinute);
        }
      }
    }

    spO2 = calculateSpO2Simple(redValue, irValue);

    tft.fillRect(90, 20, 60, 16, ST7735_BLACK); 
    tft.setTextSize(1);
    tft.setCursor(90, 20);
    tft.setTextColor(ST7735_MAGENTA);
    tft.print("SpO2:");
    tft.print(spO2, 1);
    tft.print(" %");

    float temperatureC = particleSensor.readTemperature();
    tft.fillRect(90, 40, 60, 16, ST7735_BLACK); 
    tft.setTextSize(1);  
    tft.setCursor(90, 40);
    tft.setTextColor(ST7735_YELLOW);  
    tft.print("Temp:");
    tft.print(temperatureC, 1);
    tft.print(" C");

    irMovingAverage = (irMovingAverage * (smoothingFactor - 1) + irValue) / smoothingFactor;

    if (irValue < irMovingAverage - 300) {
      if (!isInhaling) {
        isInhaling = true;
        isExhaling = false;
        tft.fillRect(0, 50, 60, 10, ST7735_BLACK);  
        tft.setCursor(0, 50);
        tft.setTextColor(ST7735_GREEN);
        tft.print("Inhale");
        
        pixels.fill(pixels.Color(0, 255, 0)); 
        pixels.show();
      }
    } else if (irValue > irMovingAverage + 300) {
      if (!isExhaling) {
        isExhaling = true;
        isInhaling = false;
        tft.fillRect(0, 50, 60, 10, ST7735_BLACK);  
        tft.setCursor(0, 50);
        tft.setTextColor(ST7735_RED);
        tft.print("Exhale");
        
        pixels.fill(pixels.Color(255, 0, 0)); 
        pixels.show();
      }
    }
  } else {
    tft.fillRect(0, 50, 60, 10, ST7735_BLACK);
    pixels.clear();
    pixels.show();
  }

  y = calculateGraphPosition(fingerDetected ? irValue : SensorOffset, SensorOffset);
  tft.drawLine(lastx, lasty, x, y, ST7735_YELLOW);
  lasty = y;
  lastx = x;
  x += TRACE_SPEED;

  if (x > 158) { 
    tft.fillRect(1, TRACE_MIN_Y, 158, TRACE_HEIGHT, ST7735_BLACK); 
    x = 1;
    lastx = x;
  }

  updateNeopixelHeartbeat();
}

void updateNeopixelHeartbeat() {
  unsigned long currentMillis = millis();

  if (fingerDetected && currentMillis - lastHeartBeatUpdate >= heartbeatInterval) {
    lastHeartBeatUpdate = currentMillis;

    if (increasingBrightness) {
      heartBrightness += 15;
      if (heartBrightness >= 255) increasingBrightness = false;
    } else {
      heartBrightness -= 15;
      if (heartBrightness <= 0) increasingBrightness = true;
    }

    uint32_t color = pixels.Color(255, 105, 180); // True pink color
    uint32_t pulsingColor = pixels.Color((heartBrightness * 255) / 255, (heartBrightness * 105) / 255, (heartBrightness * 180) / 255);
    
    pixels.fill(pulsingColor);
    pixels.show();
  } else if (!fingerDetected) {
    pixels.clear(); 
    pixels.show();
  }
}

int calculateGraphPosition(long irValue, int &SensorOffset) {
  int32_t Diff = irValue - SensorOffset;
  Diff /= SCALING;

  if (Diff < -HALF_TRACE_HEIGHT)
    SensorOffset += (SCALING * (abs(Diff) - 32));
  if (Diff > HALF_TRACE_HEIGHT)
    SensorOffset += (SCALING * (abs(Diff) - 32));

  int y = Diff + (TRACE_MIDDLE_Y_POSITION - HALF_TRACE_HEIGHT);
  y += TRACE_HEIGHT / 4;
  return constrain(y, TRACE_MIN_Y, TRACE_MAX_Y);
}

boolean checkForBeat(long irValue) {
  static long prevIrValue = 0;
  static boolean prevState = false;

  boolean beatDetected = (irValue > prevIrValue + 20) && prevState && (millis() - lastBeat > 300);
  prevIrValue = irValue;
  prevState = (irValue > 5000);

  return beatDetected;
}

float calculateSpO2Simple(long redValue, long irValue) {
  float ratio = (float)redValue / irValue;
  float spO2 = 110.0 - (25.0 * ratio);
  return constrain(spO2, 0.0, 100.0);
}