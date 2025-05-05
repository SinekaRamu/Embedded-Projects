#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// GPS setup
SoftwareSerial gpsSerial(4, 2); // GPS TX → D4 (Nano RX)
TinyGPSPlus gps;

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change I2C address if needed

// Motor control pins
const int enablePin = 9;   // PWM speed control
const int forwardPin = 5;  // Motor forward control

// IR sensor pin
const int irSensorPin = 3;

// Switch for manual/auto mode
const int manualSw = 12;
bool manualMode = false;
bool gpsFixAcquired = false;

// Manual mode coordinates (example)
const float manualLat = 11.3122;
const float manualLng = 77.6423;

// Serial communication
unsigned long lastSerialCheck = 0;
const int serialCheckInterval = 500;

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(enablePin, OUTPUT);
  pinMode(forwardPin, OUTPUT);
  pinMode(irSensorPin, INPUT);
  pinMode(manualSw, INPUT_PULLUP);

  showSplashScreen();
}

void loop() {
  // Check mode switch with debounce
  checkModeSwitch();

  if (!manualMode) {
    handleAutoMode();
  } else {
    handleManualMode();
  }

  // Movement logic
  if (gpsFixAcquired) {  
    analogWrite(enablePin, 100);
    digitalWrite(forwardPin, HIGH);

    if (digitalRead(irSensorPin) == LOW) {
      handleCrackDetection();
    } else {
      updateMovementDisplay();
    }
  } else {
    handleWaitingForGPS();
  }
}

// ========== Helper Functions ==========

void showSplashScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Railway Track ");
  lcd.setCursor(0, 1);
  lcd.print("Crack Detection");
  delay(2000);
  updateDisplay("GPS Location", "Finding.....", false);
  delay(2000);
}

void checkModeSwitch() {
  static unsigned long lastSwitchTime = 0;
  static bool lastSwitchState = HIGH;
  
  if (millis() - lastSwitchTime > 50) { // Debounce delay
    bool currentState = digitalRead(manualSw);
    if (currentState != lastSwitchState) {
      manualMode = (currentState == LOW);
      if (manualMode) {
        gpsFixAcquired = true;
        updateDisplay("MANUAL MODE", "Ready", true);
      } else {
        gpsFixAcquired = false;
        updateDisplay("AUTO MODE", "Acquiring GPS...", true);
      }
      lastSwitchState = currentState;
    }
    lastSwitchTime = millis();
  }
}

void handleAutoMode() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (!gpsFixAcquired && gps.location.isValid()) {
    gpsFixAcquired = true;
    updateDisplay("GPS Location", "Captured!", true);
    delay(1000);
  }
}

void handleManualMode() {
  static unsigned long lastManualUpdate = 0;
  if (millis() - lastManualUpdate > 1000) {
    updateDisplay("MANUAL MODE", "Moving Forward", false);
    lastManualUpdate = millis();
  }
}

void updateMovementDisplay() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    updateDisplay(manualMode ? "MANUAL MODE" : "AUTO MODE", 
                 "Moving Forward", false);
    lastUpdate = millis();
  }
}

void handleWaitingForGPS() {
  analogWrite(enablePin, 0);
  digitalWrite(forwardPin, LOW);
  
  static unsigned long lastGPSUpdate = 0;
  if (millis() - lastGPSUpdate > 1000) {
    updateDisplay("Waiting GPS Fix", "No Movement", true);
    lastGPSUpdate = millis();
  }
}

void updateDisplay(String line1, String line2, bool clearScreen) {
  static String currentLine1 = "";
  static String currentLine2 = "";
  
  if (clearScreen || (line1 != currentLine1)) {
    lcd.setCursor(0, 0);
    lcd.print(line1 + "                ");
    lcd.setCursor(0, 0);
    lcd.print(line1);
    currentLine1 = line1;
  }
  
  if (clearScreen || (line2 != currentLine2)) {
    lcd.setCursor(0, 1);
    lcd.print(line2 + "                ");
    lcd.setCursor(0, 1);
    lcd.print(line2);
    currentLine2 = line2;
  }
}

void handleCrackDetection() {
  // Stop motor
  analogWrite(enablePin, 0);
  digitalWrite(forwardPin, LOW);

  // Show detection message
  updateDisplay("Crack Detected!", "Processing...", true);
  delay(1000);

  // Send location data
  if (!manualMode && gps.location.isValid()) {
    sendLocationData(gps.location.lat(), gps.location.lng());
  } else {
    sendLocationData(manualLat, manualLng);
  }

  // Wait for confirmation from Pi
  waitForConfirmation();

  // Resume movement
  analogWrite(enablePin, 100);
  digitalWrite(forwardPin, HIGH);
}

void sendLocationData(float lat, float lng) {
  Serial.print("Lat: ");
  Serial.print(lat, 6);
  Serial.print(" | Lng: ");
  Serial.println(lng, 6);
  
  // Display coordinates
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lat:");
  lcd.print(String(lat).substring(0, 11));
  lcd.setCursor(0, 1);
  lcd.print("Lng:");
  lcd.print(String(lng).substring(0, 11));
  // char buffer[17];
  // snprintf(buffer, 17, "Lat:%.4f", lat);
  // updateDisplay(buffer, "", true);
  // snprintf(buffer, 17, "Lng:%.4f", lng);
  // updateDisplay(currentTopLine(), buffer, false);
  delay(3000);
}

void waitForConfirmation() {
  unsigned long startTime = millis();
  const unsigned long timeout = 10000; // 10 second timeout
  
  updateDisplay("Waiting for", "Pi confirmation", true);
  
  while (millis() - startTime < timeout) {
    if (Serial.available()) {
      String response = Serial.readStringUntil('\n');
      response.trim();
      
      if (response == "CONFIRMED") {
        updateDisplay("Pi confirmed", "Resuming...", true);
        delay(1000);
        return;
      }
    }
    delay(100);
  }
  
  updateDisplay("Timeout!", "Resuming...", true);
  delay(1000);
}

String currentTopLine() {
  // Helper function to get current top line
  // (Would need to track this in a real implementation)
  return "";
}