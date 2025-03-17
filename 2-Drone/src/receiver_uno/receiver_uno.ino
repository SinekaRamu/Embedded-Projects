// 3 = motor 1 (right front CCW).
// 5 = motor 2 (right rear CW).
// 6 = motor 3 (left rear CCW).
// 9 = motor 4 (left front CW).

#include <SoftwareSerial.h>
#include <Servo.h>
#include <Wire.h>

SoftwareSerial HC12(10, 11); // HC-12 TX Pin, HC-12 RX Pin

Servo motor1, motor2, motor3, motor4;
const char *myDroneID = "A"; // Unique ID for this receiver
bool isFlying = false;
unsigned long lastSendTime = 0; 

// MPU6050 Address
#define MPU 0x68
float accX, accY, accZ;
float gyroX, gyroY;
float accAngleX, accAngleY, angleX, angleY;
float biasX = 0, biasY = 0;

void setup()
{
    Serial.begin(9600);
    HC12.begin(9600);

    motor1.attach(3);
    motor2.attach(5);
    motor3.attach(6);
    motor4.attach(9);

    // Send min throttle signal to arm ESCs
    motor1.writeMicroseconds(1000);
    motor2.writeMicroseconds(1000);
    motor3.writeMicroseconds(1000);
    motor4.writeMicroseconds(1000);
    delay(3000);
    Serial.println("Drone Ready!");

    pinMode(13, OUTPUT);

    // MPU6050 Initialization
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission();

    // **Step 1: Calculate Bias Offset (Drone at Rest)**
    Serial.println("Calibrating MPU6050... Keep the drone still.");
    for (int i = 0; i < 100; i++)
    {
        readMPU6050(); // Read MPU values multiple times
        biasX += accAngleX;
        biasY += accAngleY;
        delay(10);
    }
    biasX /= 100; // Average bias value
    biasY /= 100;
    Serial.print("BiasX: ");
    Serial.print(biasX);
    Serial.print(" | BiasY: ");
    Serial.println(biasY);
}

// Stop motors
void stopMotors()
{
    int throttle;
    // Gradually decrease throttle (simulate landing)
    for (throttle = 1100; throttle >= 1000; throttle -= 50)
    {
        motor1.writeMicroseconds(throttle);
        motor2.writeMicroseconds(throttle);
        motor3.writeMicroseconds(throttle);
        motor4.writeMicroseconds(throttle);

        Serial.print("Throttle: ");
        Serial.println(throttle);
        delay(5000);
    }

    Serial.println("Motors Stopped");
}

void startMotors()
{
    int throttle;

    // Gradually increase throttle for all motors
    for (throttle = 1000; throttle <= 1100; throttle += 50)
    {
        motor1.writeMicroseconds(throttle);
        motor2.writeMicroseconds(throttle);
        motor3.writeMicroseconds(throttle);
        motor4.writeMicroseconds(throttle);

        Serial.print("Throttle: ");
        Serial.println(throttle);
        delay(2000);
    }
}

void loop()
{
    readMPU6050();  // Read sensor data
    // sendSensorData();
    if (HC12.available())
    {
        String receivedMessage = HC12.readStringUntil('\n'); // Read full message
        Serial.print("Received: ");
        Serial.println(receivedMessage);

        if (receivedMessage.startsWith(myDroneID))
        {                                             // Check if message is for this drone
            char command = receivedMessage.charAt(1); // Get the command (1 or 2)

            if (command == '1'  && !isFlying)
            {
                isFlying = true;
                Serial.println("Taking Off...");
                digitalWrite(13, HIGH);
                startMotors();

                HC12.print(myDroneID);
                HC12.print("Hi,X:");
                HC12.print(angleX);
                HC12.print(",Y:");
                HC12.println(angleY);
            }
            else if (command == '2' && isFlying)
            {
                isFlying = false;
                Serial.println("Landing...");
                digitalWrite(13, LOW);
                stopMotors();
                Serial.println(String(myDroneID) + "BYE");
                HC12.print(myDroneID);
                HC12.println("BYE");
            }
        }
    }

    // Send MPU data every 1 second while flying
    if (isFlying && millis() - lastSendTime >= 1000)
    {
        sendSensorData();
        lastSendTime = millis();
    }
}

// Function to send MPU6050 data over HC-12
void sendSensorData()
{
    Serial.print("X: ");
    Serial.print(angleX);
    Serial.print(" | Y: ");
    Serial.println(angleY);

    HC12.print(myDroneID);
    HC12.print("Hi,X:");
    HC12.print(angleX);
    HC12.print(",Y:");
    HC12.println(angleY);
}

// Function to read MPU6050 data
void readMPU6050()
{
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);

    accX = Wire.read() << 8 | Wire.read();
    accY = Wire.read() << 8 | Wire.read();
    accZ = Wire.read() << 8 | Wire.read();

    accAngleX = atan2(accY, accZ) * 180 / PI;
    accAngleY = atan2(accX, accZ) * 180 / PI;

    // **Step 2: Apply Bias Correction**
    angleX = (accAngleX - biasX);
    angleY = (accAngleY - biasY);
}