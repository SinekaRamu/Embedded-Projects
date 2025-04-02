#include "BluetoothSerial.h"

String device_name = "ESP32-BT-Slave";

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT; // RX, TX

#define ENA 16 // Speed control for Left motors
#define ENB 17 // Speed control for Right motors
#define IN1 5  // Left motor forward
#define IN2 18 // Left motor backward
#define IN3 19 // Right motor forward
#define IN4 21 // Right motor backward

void setup()
{
    Serial.begin(115200);
    SerialBT.begin(device_name);
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
}

void loop()
{
    if (SerialBT.available())
    {

        char val = SerialBT.read();
        if (val == 'f')
        {
            Serial.println("Forward");
            moveForward();
        }
        else if (val == 'b')
        {
            Serial.println("Backward");
            moveBackward();
        }
        else if (val == 'l')
        {
            Serial.println("left");
            turnLeft();
        }
        else if (val == 'r')
        {
            Serial.println("right");
            turnRight();
        }
        else if (val == 's')
        {
            Serial.println("stop");
            stopMotors();
        }
    }
}

void moveForward()
{
    analogWrite(ENA, 250); // Set motor speed (0-255)
    analogWrite(ENB, 250);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void moveBackward()
{
    analogWrite(ENB, 100);
    analogWrite(ENB, 100);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void turnLeft()
{
    analogWrite(ENB, 150);
    analogWrite(ENB, 150);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void turnRight()
{
    analogWrite(ENB, 150);
    analogWrite(ENB, 150);
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