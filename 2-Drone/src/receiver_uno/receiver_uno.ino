// 3 = motor 1 (right front CCW).
// 5 = motor 2 (right rear CW).
// 6 = motor 3 (left rear CCW).
// 9 = motor 4 (left front CW).

#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial HC12(10, 11); // HC-12 TX Pin, HC-12 RX Pin

Servo motor1, motor2, motor3, motor4;
const char *myDroneID = "A"; // Unique ID for this receiver

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
        delay(1000);
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
        delay(1000);
    }
}

void loop()
{

    if (HC12.available())
    {
        String receivedMessage = HC12.readStringUntil('\n'); // Read full message
        Serial.print("Received: ");
        Serial.println(receivedMessage);

        if (receivedMessage.startsWith(myDroneID))
        {                                             // Check if message is for this drone
            char command = receivedMessage.charAt(1); // Get the command (1 or 2)

            if (command == '1')
            {
                Serial.println("Taking Off...");
                digitalWrite(13, HIGH);
                startMotors();
                HC12.print(myDroneID);
                HC12.println("ACK2"); // Send acknowledgment
            }
            else if (command == '2')
            {
                Serial.println("Landing...");
                stopMotors();
                digitalWrite(13, LOW);
                Serial.println(String(myDroneID) + "ACK2");
                HC12.print(myDroneID);
                HC12.println("ACK2");
            }
        }
    }
    delay(1000);
}
