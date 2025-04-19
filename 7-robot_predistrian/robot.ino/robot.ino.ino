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

const int trigPin = 2;
const int echoPin = 4;
long duration;
int distance;

// setting PWM properties
const int freq = 1000;
const int resolution = 8;

void setup()
{
    Serial.begin(115200);
    SerialBT.begin("ESP32-BT-Robot");

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

    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);  // Sets the echoPin as an Input

    // Set motor control pins as outputs
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    ledcAttach(ENA, freq, resolution);
    ledcAttach(ENB, freq, resolution);

    // Initialize motors in stopped state
    stopMotors();

    display.setCursor(0, 12);
    display.print("Bluetooth name:");
    display.setCursor(0, 24);
    display.println("ESP32-BT-Robot");
    display.display();

    Serial.println("Bluetooth Robot Ready!");
}

void updateDisplay(String message)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Status:");
    display.println(message);
    display.display();
}

void loop()
{

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    distance = duration * 0.034 / 2;
    // Prints the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.println(distance);
    delay(50);

    if (distance <= 10)
    {
        stopMotors();
    }

    if (SerialBT.available())
    {
        char cmd = SerialBT.read();
        Serial.println(cmd);

        switch (cmd)
        {
        case 'w':
            updateDisplay("Moving Forward");
            moveForward();
            break;
        case 's':
            updateDisplay("Moving Backward");
            moveBackward();
            break;
        case 'a':
            updateDisplay("Moving Left");
            turnLeft();
            break;
        case 'd':
            updateDisplay("Moving Right");
            turnRight();
            break;
        case 'x':
            updateDisplay("Moving Stop");
            stopMotors();
            break;
        }
    }
}

void moveForward()
{
    ledcWrite(ENA, 120); // Left motor speed (0-255)
    ledcWrite(ENB, 120); // Right motor speed (0-255)
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void moveBackward()
{
    ledcWrite(ENA, 80);
    ledcWrite(ENB, 80);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void turnLeft()
{
    ledcWrite(ENA, 80);
    ledcWrite(ENB, 80);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void turnRight()
{
    ledcWrite(ENA, 80);
    ledcWrite(ENB, 80);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void stopMotors()
{
    ledcWrite(ENA, 0);
    ledcWrite(ENB, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}