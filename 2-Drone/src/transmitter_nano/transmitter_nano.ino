#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

#define vibrationSensor 4

SoftwareSerial HC12(10, 11); // HC-12 TX Pin, HC-12 RX Pin
LiquidCrystal_I2C lcd(0x27, 16, 2);

int lastState = LOW;
const char *droneID = "A";

void setup()
{
    Serial.begin(9600);
    HC12.begin(9600);

    pinMode(vibrationSensor, INPUT);

    lcd.init();
    lcd.backlight();

    lcd.setCursor(0, 0);
    lcd.print(" Drone System");
    delay(2000);
}

bool checkSensor()
{
    unsigned long startTime = millis();
    int currentState = digitalRead(vibrationSensor);

    // Monitor the sensor for 2 seconds
    while (millis() - startTime < 2000)
    {
        if (digitalRead(vibrationSensor) != currentState)
        {
            Serial.println("State changed within 2 sec, canceling command.");
            break; // Exit if state changes
        }
        delay(100); // Stability check
    }
    return currentState;
}

void countDown()
{
    lcd.setCursor(0, 0);
    lcd.print(" Countdown: ");

    for (int i = 10; i > 0; i--)
    {
        lcd.setCursor(12, 0);
        lcd.print("    "); // Clear previous number
        lcd.setCursor(12, 0);
        lcd.print(i);
        delay(1000);
    }
}

bool waitForACK()
{
    unsigned long startTime = millis();
    while (millis() - startTime < 5000)
    { // Timeout after 5 seconds

        if (HC12.available())
        {
            String ack = HC12.readStringUntil('\n');
            Serial.println("ACK Received: " + ack);

            if (ack.startsWith(droneID))
            { // Check if message is for this drone
                char command = ack.charAt(1);

                if (command == 'A')
                {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("ACK Received");
                    delay(2000);
                    return true;
                }
            }
        }
        delay(100);
    }
    Serial.println("ACK Timeout");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ACK Timeout");
    return false;
}

void SendDataToDrone(int currentState)
{
    if (currentState == HIGH && lastState == LOW)
    {
        HC12.print(droneID);
        HC12.println("1"); // Send Takeoff command
        Serial.println("Sent: 1 (Takeoff)");

        if (!waitForACK())
        {
            lcd.setCursor(0, 1);
            lcd.print(" Failed");
        }
        else
        {
            lcd.setCursor(0, 1);
            lcd.print(" Taking Off");
        }
    }
    else if (currentState == LOW && lastState == HIGH)
    {
        HC12.print(droneID);
        HC12.println("2"); // Send Land command
        Serial.println("Sent: 2 (Land)");

        if (!waitForACK())
        {
            lcd.print(0, 1);
            lcd.print(" Failed");
        }
        else
        {
            lcd.setCursor(0, 1);
            lcd.print("Status: Landing");
        }
    }
    else if (currentState == HIGH)
    {
        lcd.setCursor(0, 1);
        lcd.print("Holding Altitude");
    }
    else
    {
        lcd.setCursor(4, 1);
        lcd.print("No Issue");
    }
    delay(1000);
}

void loop()
{

    int currentState = checkSensor();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Checking...");

    SendDataToDrone(currentState);

    countDown();
    lastState = currentState;
}
