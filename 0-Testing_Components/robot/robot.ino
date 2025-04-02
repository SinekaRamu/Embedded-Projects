#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif

BluetoothSerial SerialBT;

// Motor control pins
#define ENA 16  // Left motor speed control (must be PWM capable)
#define ENB 17  // Right motor speed control (must be PWM capable)
#define IN1 5   // Left motor forward
#define IN2 18  // Left motor backward
#define IN3 19  // Right motor forward
#define IN4 21  // Right motor backward

// setting PWM properties
const int freq = 1000;
const int resolution = 8;
 
void setup() {
    Serial.begin(115200);
    SerialBT.begin("ESP32-BT-Robot");

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

    Serial.println("Bluetooth Robot Ready!");
}

void loop() {
    if (SerialBT.available()) {
        char cmd = SerialBT.read();
        Serial.println(cmd);

        switch (cmd) {
            case 'f': moveForward(); break;
            case 'b': moveBackward(); break;
            case 'l': turnLeft(); break;
            case 'r': turnRight(); break;
            case 's': stopMotors(); break;
        }
    }
}`12q 

void moveForward() {
    ledcWrite(ENA, 200); // Left motor speed (0-255)
    ledcWrite(ENB, 200); // Right motor speed (0-255)
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void moveBackward() {
    ledcWrite(ENA, 200);
    ledcWrite(ENB, 200);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void turnLeft() {
    ledcWrite(ENA, 150);
    ledcWrite(ENB, 150);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void turnRight() {
    ledcWrite(ENA, 150);
    ledcWrite(ENB, 150);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void stopMotors() {
    ledcWrite(ENA, 0);
    ledcWrite(ENB, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}