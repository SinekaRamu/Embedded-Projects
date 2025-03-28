#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BUZZER D0
#define RED D6
#define GREEN D5

// LCD Setup (16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

bool gasDetected = false;
unsigned long previousMillis = 0;
const unsigned long interval = 500; // 500ms interval for buzzer

void setup()
{
    Serial.begin(115200);

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("System Initializing");
    lcd.setCursor(0, 1);
    lcd.print("    HAND BOT");
    delay(5000);
    lcd.clear();

    pinMode(A0, INPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(RED, OUTPUT);
    pinMode(BUZZER, OUTPUT);

    Serial.println("System initialized");
}

void loop()
{
    int gas = analogRead(A0);
    int percentage = map(gas, 0, 1024, 0, 100);
    Serial.println(gas);
    Serial.println(percentage);

    lcd.setCursor(0, 0);
    lcd.print("sensor: ");
    lcd.setCursor(9, 0);
    lcd.print("    "); // Clear old data
    lcd.setCursor(9, 0);
    lcd.print(percentage);
    lcd.setCursor(13, 0);
    lcd.print("%");

    // Gas detection and buzzer logic
    if (percentage > 30)
    {
        gasDetected = true;
        digitalWrite(RED, HIGH);
        digitalWrite(GREEN, LOW);
        lcd.setCursor(0, 1);
        lcd.print(" Gas detected!!!");

        // Buzzer ON-OFF at 500ms intervals
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {
            previousMillis = currentMillis;
            digitalWrite(BUZZER, !digitalRead(BUZZER)); // Toggle Buzzer state
        }
    }
    else
    {
        gasDetected = false;
        digitalWrite(GREEN, HIGH);
        digitalWrite(RED, LOW);
        digitalWrite(BUZZER, LOW); // Ensure buzzer is off
        lcd.setCursor(0, 1);
        lcd.print("system is normal ");
    }

    delay(1000);
}
