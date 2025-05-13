#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

#define vibrationSensor 4

SoftwareSerial HC12(10, 11); // HC-12 TX Pin, HC-12 RX Pin
LiquidCrystal_I2C lcd(0x27, 16, 2);

int lastState = LOW;
const char *droneID = "A";
bool isFlying = false;
unsigned long lastSensorCheck = 0;

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

void displayFlightData(float pitch, float roll)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pitch: ");
    lcd.print(pitch, 2);

    lcd.setCursor(0, 1);
    lcd.print("Roll: ");
    lcd.print(roll, 2);
    delay(2000);
}

void extractMPUData(String receivedData)
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
        displayFlightData(pitch, roll);
    }
}

void receiveMPUData()
{
    if (HC12.available())
    {
        String receivedData = HC12.readStringUntil('\n');
        lcd.clear();
        lcd.print(receivedData);

        if (receivedData.startsWith(droneID))
        {
            if (receivedData.indexOf("Hi") != -1)
            {
                if (!isFlying)
                {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Hi, Drone");
                    delay(500);
                }
                isFlying = true;
                extractMPUData(receivedData);
            }
            else if (receivedData.indexOf("BYE") != -1)
            {
                isFlying = false;
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("BYE");
                lcd.setCursor(0, 1);
                lcd.print("Landed");
                delay(1000);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print(" Drone System ");

            }
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
        lcd.setCursor(0, 1);
        lcd.print("sent Takeoff");
        delay(1000);
    }
    else if (currentState == LOW && lastState == HIGH)
    {
        HC12.print(droneID);
        HC12.println("2");
        Serial.println("Sent: 2 (Land)");
        lcd.setCursor(0, 1);
        lcd.print("sent stop    ");
        delay(1000);
    }else if (currentState == HIGH)
    {
        lcd.setCursor(0, 1);
        lcd.print("Holding Altitude");
    }

    // Show sensor status only if drone is flying
    if (isFlying)
    {
        lcd.setCursor(0, 1);
        lcd.print("No Issue       ");
    }

    lastState = currentState;
}

void loop()
{
    receiveMPUData();

    if (millis() - lastSensorCheck >= 5000)
    {
        lastSensorCheck = millis();
        int currentState = digitalRead(vibrationSensor);
        lcd.setCursor(0,1);
        lcd.print("Checking....    ");
        SendDataToDrone(currentState);
    }
}
