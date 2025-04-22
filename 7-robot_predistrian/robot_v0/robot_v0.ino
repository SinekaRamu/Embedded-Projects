#include <HardwareSerial.h>
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
#define ENA 25 // Left motor speed control (must be PWM capable)
#define ENB 26 // Right motor speed control (must be PWM capable)
#define IN1 5  // Left motor forward
#define IN2 18 // Left motor backward
#define IN3 19 // Right motor forward
#define IN4 23 // Right motor backward

// Ultrasonic pins
#define TRIG_R 4      // Rear Trigger
#define ECHO_R 15     // Rear Echo
#define TRIG_F 32     // Front Trigger
#define ECHO_F 35     // Front Echo

// setting PWM properties
const int freq = 1000;
const int resolution = 8;

volatile bool rearObstacle = false;
volatile bool frontObstacle = false;

volatile unsigned long echo_start = 0;
volatile unsigned long echo_end = 0;

char lastCmd = 'x'; // tracks last movement command
bool wasStoppedByObstacle = false;

void triggerUltrasonic(int trigPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
}

int readDistance(int echoPin) {
  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  if (duration == 0) return 1000; // No echo received
  int distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  SerialBT.begin("ESP32-BT-Robot");
  
  pinMode(TRIG_R, OUTPUT);
  pinMode(ECHO_R, INPUT);
  pinMode(TRIG_F, OUTPUT);
  pinMode(ECHO_F, INPUT);

  // Set motor control pins as outputs
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcAttach(ENA, freq, resolution);
  ledcAttach(ENB, freq, resolution);

  ledcWrite(ENA, 120);
  ledcWrite(ENB, 120);

  stopMotors();
  setupOled();

}

void setupOled(){ 
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("OLED failed"));
        while(true);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ESP32 Ready");
    display.display();
    delay(5000);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Bluetooth name:");
    display.setCursor(0, 12);
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
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void moveBackward()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void turnLeft()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void turnRight()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void stopMotors()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}

void loop() {
  //Trigger and read rear sensor
  triggerUltrasonic(TRIG_R); 
  int distR = readDistance(ECHO_R);
  rearObstacle = (distR <= 10);
  
  // Trigger and read front sensor
  triggerUltrasonic(TRIG_F);
  int distF = readDistance(ECHO_F);
  frontObstacle = (distF <= 10);

  if (rearObstacle && lastCmd == 's') {
    stopMotors();
    updateDisplay("Rear Obstacle");
    delay(2000);
    wasStoppedByObstacle = true;
    return;
  }

  if (frontObstacle && lastCmd == 'w') {
    stopMotors();
    updateDisplay("Front Obstacle");
    delay(1000);
    
    // Send detection trigger to Pi
    Serial2.println("DETECT");  // Send as newline-terminated string 
    delay(5000);
    
    wasStoppedByObstacle = true;
    return;
  }

   if (wasStoppedByObstacle && !rearObstacle && !frontObstacle && lastCmd != 'x') {
    switch (lastCmd) {
      case 'w': moveForward(); updateDisplay("Resumed Forward"); break;
      case 's': moveBackward(); updateDisplay("Resumed Backward"); break;
      case 'a': turnLeft(); updateDisplay("Resumed Left"); break;
      case 'd': turnRight(); updateDisplay("Resumed Right"); break;
    }
    wasStoppedByObstacle = false;
  }

  // Bluetooth command handler
  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    lastCmd = cmd;
    Serial.println(cmd);

    switch (cmd) {
      case 'w': updateDisplay("Forward"); moveForward(); break;
      case 's': updateDisplay("Backward"); moveBackward(); break;
      case 'a': updateDisplay("Left"); turnLeft(); break;
      case 'd': updateDisplay("Right"); turnRight(); break;
      case 'x': updateDisplay("Stopped"); stopMotors(); break;
    }
  }
  delay(100);
}