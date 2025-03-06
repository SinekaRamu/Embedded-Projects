#include "BluetoothSerial.h"
#include "esp_bt.h"

#include <Wire.h>
#include "MAX30105.h"  // MAX30102 library (use MAX3010X)
#include "Adafruit_Sensor.h"
#include "heartRate.h"
#include <Ticker.h>  // Built-in Library for ESP32

#define MQ4_PIN  34  // Methane sensor (MQ-4) connected to GPIO 34
#define MQ7_PIN  35  // Carbon Monoxide sensor (MQ-7) connected to GPIO 35
#define BUZZER 4

#define METHANE_THRESHOLD 900  // ppm - Dangerous level
#define CO_THRESHOLD 200       // ppm - Dangerous level
#define BPM_LOW 50             // Low Heart Rate
#define BPM_HIGH 120           // High Heart Rate

BluetoothSerial SerialBT;  // Create Bluetooth Object
uint8_t address[6] = {0x98, 0xDA, 0x50, 0x03, 0x08, 0x14};

void setBTpin(const char* pin) {
  esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED, strlen(pin), (uint8_t*)pin);
}

Ticker timer;  // Create Ticker Object
volatile bool timerFlag = false;  // Flag for Timer Trigger

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

void timerISR() {
  timerFlag = true;  // Set the flag every second
}

void setup() {
    Serial.begin(9600);
    // Release BLE memory (since we're using Classic Bluetooth)
    esp_bt_controller_mem_release(ESP_BT_MODE_BLE);

    SerialBT.begin("ESP32-BT-MASTER", true);  // Master mode
    setBTpin("1234");
    Serial.println("ESP32 in Master Mode");
    delay(1000);

    bool connected = SerialBT.connect(address);
    if (!connected) {
      Serial.println("Failed to connect, restarting ESP32...");
      delay(2000);
      esp_restart();  // Restart ESP32 if connection fails
    }else {
      Serial.println("Connected Successfully!");
    } 

    Wire.begin(); // Initialize I2C
    pinMode(BUZZER, OUTPUT);

    timer.attach(1.5, timerISR);  // Call ISR every 1 second
    Serial.println("Timer Initialized");

    // Initialize MAX30102
    if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
        Serial.println("MAX30102 not found. Check connections!");
        while (1);
    }
    
    particleSensor.setup(); //Configure sensor with default settings
    particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
    particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
    Serial.println("Sensors initialized successfully!");

    if (SerialBT.hasClient()) {
      Serial.println("Bluetooth Connected");
      SerialBT.println("Hello SAFEBOT");
    } else {
      Serial.println("Bluetooth Not Connected");
    }
}

void loop() {

    long irValue = particleSensor.getIR();
    
    if (checkForBeat(irValue) == true)
    {
      //We sensed a beat!
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60 / (delta / 1000.0);

        if (beatsPerMinute < 255 && beatsPerMinute > 20)
        {
          rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
          rateSpot %= RATE_SIZE; //Wrap variable

          //Take average of readings
          beatAvg = 0;
          for (byte x = 0 ; x < RATE_SIZE ; x++)
            beatAvg += rates[x];
          beatAvg /= RATE_SIZE;
        }
    }
    // Serial.print(", Avg BPM=");
    // Serial.print(beatAvg);

    // if (irValue < 50000)
    // Serial.print(" No finger?");
    // Serial.println();
    
    if (timerFlag) {
      timerFlag = false;  // Reset the flag

      // Read MQ-4 (Methane Gas)
      int mq4_value = analogRead(MQ4_PIN);
      float methane_concentration = map(mq4_value, 0, 4095, 0, 1000); // Rough ppm estimation

      // Read MQ-7 (Carbon Monoxide Gas)
      int mq7_value = analogRead(MQ7_PIN);
      float co_concentration = map(mq7_value, 0, 4095, 0, 1000); // Rough ppm estimation

      // Print sensor values
      SerialBT.print("Methane (MQ-4): ");
      SerialBT.print(methane_concentration);
      SerialBT.println(" ppm");

      SerialBT.print("Carbon Monoxide (MQ-7): ");
      SerialBT.print(co_concentration);
      SerialBT.println(" ppm");

      SerialBT.print("Heart Rate: ");
      SerialBT.print(beatAvg);
      SerialBT.println(" bpm");
      // Check Thresholds
      if (methane_concentration > METHANE_THRESHOLD) {
        String message = "🔥 ALERT: Methane Gas Detected!";
        SerialBT.println(message);  // Send Message
        digitalWrite(BUZZER, HIGH);
      }

      if (co_concentration > CO_THRESHOLD) {
          String message = "⚠️ ALERT: Carbon Monoxide Detected!";
          SerialBT.println(message);  // Send Message
          digitalWrite(BUZZER, HIGH);
      }

      if (beatAvg > BPM_HIGH || beatAvg < BPM_LOW) {
          String message = "🚨 ALERT: Abnormal Heart Rate!";
          SerialBT.println(message);  // Send Message
          digitalWrite(BUZZER, HIGH);
      }
      // Reset Buzzer
      digitalWrite(BUZZER, LOW);
    }
       
}

