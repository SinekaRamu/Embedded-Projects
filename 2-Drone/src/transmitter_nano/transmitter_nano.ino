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
        Serial.println("Received: " + receivedData);

        if (receivedData.startsWith(droneID))
        {
            if (receivedData.indexOf("Hi") != -1) 
            {
                isFlying = true; // Drone is in flight
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Flying...");
                delay(1000);
                extractMPUData(receivedData);
            }
            else if (receivedData.indexOf("BYE") != -1) 
            {
                isFlying = false; // Drone has landed
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Landed");
                delay(2000);
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
        receiveMPUData();
    }
    else if (currentState == LOW && lastState == HIGH)
    {
        HC12.print(droneID);
        HC12.println("2");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Landing...");
        receiveMPUData();
    }else if (currentState == HIGH)
    {
        lcd.setCursor(0, 1);
        lcd.print("Holding Altitude");
    }
    else{
        lcd.setCursor(0, 1);
        lcd.print("  No Issue  ");
    }
    delay(1000);
    lastState = currentState;
}

void loop()
{
    receiveMPUData();

    // **2. Check sensor value every 5 seconds**
    if (millis() - lastSensorCheck >= 10000)  
    {
        lastSensorCheck = millis(); // Update last checked time
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("checking....");
        int currentState = digitalRead(vibrationSensor); // Read sensor
        SendDataToDrone(currentState); // Execute based on sensor data
    }
}

