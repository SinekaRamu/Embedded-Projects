#include <Wire.h>
#include "MAX30105.h" // MAX30102 library (use MAX3010X)
#include "Adafruit_Sensor.h"
#include "heartRate.h"
#include <Ticker.h> // Built-in Library for ESP32
#include <SoftwareSerial.h>

#define MQ4_PIN 34 // Methane sensor (MQ-4) connected to GPIO 34
#define MQ7_PIN 35 // Carbon Monoxide sensor (MQ-7) connected to GPIO 35
#define BUZZER 4

#define METHANE_THRESHOLD 900 // ppm - Dangerous level
#define CO_THRESHOLD 200      // ppm - Dangerous level
#define BPM_LOW 50            // Low Heart Rate
#define BPM_HIGH 120          // High Heart Rate

SoftwareSerial HC12(18, 19); // HC-12 TX Pin, HC-12 RX Pinne MQ4_PIN  34  // Methane sensor (MQ-4) connected to GPIO 34
const char *ID = "S";

Ticker timer;                    // Create Ticker Object
volatile bool timerFlag = false; // Flag for Timer Trigger

MAX30105 particleSensor;

const byte RATE_SIZE = 4; // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE];    // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

void timerISR()
{
  timerFlag = true; // Set the flag every second
}

void setup()
{
  Serial.begin(115200);
  HC12.begin(9600);

  Wire.begin(); // Initialize I2C
  pinMode(BUZZER, OUTPUT);

  timer.attach(3.0, timerISR); // Call ISR every 1 second
  Serial.println("Timer Initialized");

  // Initialize MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD))
  {
    Serial.println("MAX30102 not found. Check connections!");
    while (1)
      ;
  }

  particleSensor.setup();                    // Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);  // Turn off Green LED
  Serial.println("Sensors initialized successfully!");
}

void loop()
{
  
  long irValue = particleSensor.getIR();

  if (irValue > 50000)
  {
    if (checkForBeat(irValue) == true)
    {
      // We sensed a beat!
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute < 255 && beatsPerMinute > 20)
      {
        rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading in the array
        rateSpot %= RATE_SIZE;                    // Wrap variable

        // Take average of readings
        beatAvg = 0;
        for (byte x = 0; x < RATE_SIZE; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }
    Serial.print(", Avg BPM=");
    Serial.print(beatAvg);
  }

  if (timerFlag)
  {
    timerFlag = false; // Reset the flag

    // Read MQ-4 (Methane Gas)
    int mq4_value = analogRead(MQ4_PIN);
    float methane_concentration = map(mq4_value, 0, 4095, 0, 1000); // Rough ppm estimation

    // Read MQ-7 (Carbon Monoxide Gas)
    int mq7_value = analogRead(MQ7_PIN);
    float co_concentration = map(mq7_value, 0, 4095, 0, 1000); // Rough ppm estimation
    Serial.println("Timer Interrupt Triggered");

    // Print sensor values
    Serial.print("Methane (MQ-4): ");
    Serial.print(methane_concentration);
    Serial.println(" ppm");

    Serial.print("Carbon Monoxide (MQ-7): ");
    Serial.print(co_concentration);
    Serial.println(" ppm");

    Serial.println("----------------------");
    // Check Thresholds
    if (methane_concentration > METHANE_THRESHOLD)
    {
      Serial.println("🔥 ALERT: Methane Gas Detected!");
      digitalWrite(BUZZER, HIGH);
      HC12.print(ID);
      HC12.println("1");
    }

    if (co_concentration > CO_THRESHOLD)
    {
      Serial.println("⚠️ ALERT: Carbon Monoxide Detected!");
      digitalWrite(BUZZER, HIGH);
      HC12.print(ID);
      HC12.println("2");
    }

    if (beatAvg > BPM_HIGH || beatAvg < BPM_LOW)
    {
      Serial.println("🚨 ALERT: Abnormal Heart Rate!");
      digitalWrite(BUZZER, HIGH);
      HC12.print(ID);
      HC12.println("3");
    }
    // Reset Buzzer
    digitalWrite(BUZZER, LOW);
  }
}
