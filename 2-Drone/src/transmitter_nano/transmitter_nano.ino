#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

#define vibrationSensor 4

SoftwareSerial HC12(10, 11); // HC-12 TX Pin, HC-12 RX Pin
LiquidCrystal_I2C lcd(0x27, 16, 2);

int lastState = LOW;
const char *droneID = "A";
bool isFlying = false; // Flag to track if drone is flying

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

    while (millis() - startTime < 2000)
    {
        if (digitalRead(vibrationSensor) != currentState)
        {
            Serial.println("State changed within 2 sec, canceling command.");
            break;
        }
        delay(100);
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
        lcd.print("    ");
        lcd.setCursor(12, 0);
        lcd.print(i);
        delay(1000);
    }
}

void displayPitchRoll(float pitch, float roll)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pitch: ");
    lcd.print(pitch, 2);
    
    lcd.setCursor(0, 1);
    lcd.print("Roll: ");
    lcd.print(roll, 2);
}

bool waitForACK()
{
    unsigned long startTime = millis();
    while (millis() - startTime < 5000)
    {
        if (HC12.available())
        {
            String receivedData = HC12.readStringUntil('\n');
            Serial.println("ACK Received: " + receivedData);

            if (receivedData.startsWith(droneID))
            {
                if (receivedData.indexOf("Hi") != -1)
                {
                    isFlying = true; // Drone is flying
                    return true;
                }
                else if (receivedData.indexOf("BYE") != -1)
                {
                    isFlying = false; // Drone has landed
                    return false;
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

void extractPitchRoll(String receivedData)
{
    int xIndex = receivedData.indexOf("X:");
    int yIndex = receivedData.indexOf("Y:");

    if (xIndex != -1 && yIndex != -1)
    {
        float pitch = receivedData.substring(xIndex + 2, yIndex - 1).toFloat();
        float roll = receivedData.substring(yIndex + 2).toFloat();

        Serial.print("Pitch: ");
        Serial.print(pitch);
        Serial.print(" | Roll: ");
        Serial.println(roll);

        displayPitchRoll(pitch, roll);
    }
}

void receiveMPUData()
{
    if (HC12.available())
    {
        String receivedData = HC12.readStringUntil('\n');
        Serial.println("Received Data: " + receivedData);

        if (receivedData.startsWith(droneID))
        {
            extractPitchRoll(receivedData);
        }
    }
}

void SendDataToDrone(int currentState)
{
    if (currentState == HIGH && lastState == LOW)
    {
        HC12.print(droneID);
        HC12.println("1"); 
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
        HC12.println("2"); 
        Serial.println("Sent: 2 (Land)");

        if (!waitForACK())
        {
            lcd.setCursor(0, 1);
            lcd.print(" Failed");
        }
        else
        {
            lcd.setCursor(0, 1);
            lcd.print(" Landing");
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
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Checking...");
    int currentState = checkSensor();
    delay(2000);

    SendDataToDrone(currentState);

    countDown();
    lastState = currentState;

    if (isFlying)
    {
        receiveMPUData();
    }
}
