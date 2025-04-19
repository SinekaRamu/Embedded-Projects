#include "BluetoothSerial.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1 // Reset pin (not used)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

BluetoothSerial SerialBT;

// Motor control pins
#define ENA 16 // Left motor speed control (must be PWM capable)
#define ENB 17 // Right motor speed control (must be PWM capable)
#define IN1 5  // Left motor forward
#define IN2 18 // Left motor backward
#define IN3 19 // Right motor forward
#define IN4 23 // Right motor backward
#define echoPin 15
#define trigPin 4

// setting PWM properties
const int freq = 1000;
const int resolution = 8;

volatile unsigned long echo_start = 0;
volatile unsigned long echo_end = 0;
volatile bool objectDetected = false;

char lastCmd = 'x'; // Holds the last movement command

void IRAM_ATTR echo_isr() {
  if (digitalRead(echoPin) == HIGH) {

    echo_start = micros();
  } else {
    echo_end = micros();
    unsigned long duration = echo_end - echo_start;
    int distance = duration * 0.034 / 2;

    if (distance <= 10) {
      objectDetected = true;
    }
  }
}

void triggerUltrasonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32-BT-Robot");
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Set motor control pins as outputs
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    ledcAttach(ENA, freq, resolution);
    ledcAttach(ENB, freq, resolution);

    stopMotors();
    setupOled();
  
    attachInterrupt(digitalPinToInterrupt(echoPin), echo_isr, CHANGE);
}

void setupOled(){ 
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("OLED failed"));
        for (;;)
            ; // Don't proceed
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ESP32 Ready");
    display.display();
    delay(5000);

    display.setCursor(0, 12);
    display.print("Bluetooth name:");
    display.setCursor(0, 24);
    display.println("ESP32-BT-Robot");
    display.display();
    delay(5000);
}

void updateDisplay(String message)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Status:");
    display.println(message);
    display.display();
}

void moveForward()
{
    updateDisplay("Moving Forward");
    ledcWrite(ENA, 120); // Left motor speed (0-255)
    ledcWrite(ENB, 120); // Right motor speed (0-255)
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void moveBackward()
{
    updateDisplay("Moving Backward");
    ledcWrite(ENA, 80);
    ledcWrite(ENB, 80);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void turnLeft()
{
    updateDisplay("Turning Left");
    ledcWrite(ENA, 80);
    ledcWrite(ENB, 80);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void turnRight()
{
    updateDisplay("Turning Right");
    ledcWrite(ENA, 80);
    ledcWrite(ENB, 80);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void stopMotors()
{
    updateDisplay("Stopped");
    ledcWrite(ENA, 0);
    ledcWrite(ENB, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}

void loop() {
  triggerUltrasonic(); // Trigger periodically
  delay(100);

  if (objectDetected) {
    stopMotors();
    updateDisplay("Obstacle!");
    objectDetected = false;
  }else{
    updateDisplay("NO Obstacle!");
    delay(2000);
    // Resume previous command
  switch (lastCmd) {
    case 'w':
      moveForward();
      break;
    case 's':
      moveBackward();
      break;
    case 'a':
      turnLeft();
      break;
    case 'd':
      turnRight();
      break;
  }
  }

  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    Serial.println(cmd);

    switch (cmd) {
      case 'w':
        moveForward();
        lastCmd = 'w';
        break;
      case 's':
        moveBackward();
        lastCmd = 's';
        break;
      case 'a':
        turnLeft();
        lastCmd = 'a';
        break;
      case 'd':
        turnRight();
        lastCmd = 'd';
        break;
      case 'x':
        stopMotors();
        lastCmd = 'x';
        break;
    }

  }
}
