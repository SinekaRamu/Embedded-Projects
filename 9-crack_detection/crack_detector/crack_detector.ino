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

// Flag to indicate GPS fix
bool gpsFixAcquired = false;

// Manual mode coordinates (example)
const float manualLat = 11.3122;
const float manualLng = 77.6423;

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(enablePin, OUTPUT);
  pinMode(forwardPin, OUTPUT);
  pinMode(irSensorPin, INPUT);
  pinMode(manualSw, INPUT_PULLUP);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Railway Track ");
  lcd.setCursor(0, 1);
  lcd.print("Crack  Detection");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GPS Location");
  lcd.setCursor(0, 1);
  lcd.print("Finding.....");
  delay(3000);
}

void loop() {
  // Check mode switch
  manualMode = digitalRead(manualSw) == LOW;
  delay(50);

  if (!manualMode) {
    gpsFixAcquired = false;
    // AUTO MODE (GPS-dependent)
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }

    if (!gpsFixAcquired && gps.location.isValid()) {
      gpsFixAcquired = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("GPS Location");
      lcd.setCursor(0, 1);
      lcd.print("Captured......");
      delay(3000);
    }
  } else {
    // MANUAL MODE (bypass GPS)
    lcd.setCursor(0, 0);
    lcd.print("MANUAL MODE     ");
    delay(1000);
    gpsFixAcquired = true; // Force enable movement
  }

  // Movement logic
  if (gpsFixAcquired) {  
    // Start motor
    analogWrite(enablePin, 100);
    digitalWrite(forwardPin, HIGH);

    int irValue = digitalRead(irSensorPin);

    if (irValue == LOW) {
      handleCrackDetection();
    } else {
      // Only show movement message every second to avoid flicker
      static unsigned long lastUpdate = 0;
      if (millis() - lastUpdate > 1000) {
        lcd.setCursor(0, 1);
        lcd.print("Moving Forward ");
        lastUpdate = millis();
      }
    }
  } else {
    analogWrite(enablePin, 0);
    digitalWrite(forwardPin, LOW);
    // Waiting for GPS lock
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Waiting GPS Fix ");
    lcd.setCursor(0, 1);
    lcd.print("No Movement     ");
    delay(1000);
    
  }
}

void handleCrackDetection(){
  // Stop motor
  analogWrite(enablePin, 0);
  digitalWrite(forwardPin, LOW);

  Serial.println("Crack Detected!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Crack Detected");
  lcd.setCursor(0, 1);
  lcd.print("Location Sending...");
  delay(3000);

  if (!manualMode && gps.location.isValid()) {
    float lat = gps.location.lat();
    float lng = gps.location.lng();

    Serial.print("Lat: ");
    Serial.print(lat, 6);
    Serial.print(" | Lng: ");
    Serial.println(lng, 6);

    char latStr[16], lngStr[16];
    dtostrf(lat, 10, 6, latStr);
    dtostrf(lng, 10, 6, lngStr);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lat:");
    lcd.print(String(latStr).substring(0, 11));
    lcd.setCursor(0, 1);
    lcd.print("Lng:");
    lcd.print(String(lngStr).substring(0, 11));
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lat:");
    lcd.print(String(manualLat));
    lcd.setCursor(0, 1);
    lcd.print("Lng:");
    lcd.print(String(manualLng));

    Serial.print("Lat: ");
    Serial.print(manualLat);
    Serial.print(" | Lng: ");
    Serial.println(manualLng);
  }

  delay(5000);

  // Resume movement
  analogWrite(enablePin, 100);
  digitalWrite(forwardPin, HIGH);
}