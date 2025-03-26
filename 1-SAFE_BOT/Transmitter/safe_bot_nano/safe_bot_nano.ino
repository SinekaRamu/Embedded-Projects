#include <Wire.h>
#include "MAX30105.h"
#include "Adafruit_Sensor.h"
#include "heartRate.h"
#include <SoftwareSerial.h>
#include <avr/interrupt.h>

#define MQ4_PIN A0 // Methane sensor (MQ-4) connected to A0
#define MQ7_PIN A1 // Carbon Monoxide sensor (MQ-7) connected to A1
#define BUZZER 4

#define METHANE_THRESHOLD 700
#define CO_THRESHOLD 300
#define BPM_LOW 50
#define BPM_HIGH 120

SoftwareSerial HC12(10, 11); // HC-12 TX, RX
const char *ID = "S";

volatile bool timerFlag = false; // Flag for Timer Trigger

MAX30105 particleSensor;
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute;
int beatAvg;

// Timer1 Interrupt Service Routine
ISR(TIMER1_COMPA_vect) {
  timerFlag = true;
  digitalWrite(BUZZER, LOW);
}

void setupTimer1() {
  cli(); // Disable global interrupts

  TCCR1A = 0; // Normal operation
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC mode, prescaler 1024
  OCR1A = 46874; // 3 seconds interval (16MHz / 1024 / 3Hz - 1)
  TIMSK1 |= (1 << OCIE1A); // Enable compare match interrupt

  sei(); // Enable global interrupts
}

void setup() {
  Serial.begin(115200);
  HC12.begin(9600);

  Wire.begin();
  pinMode(BUZZER, OUTPUT);
  setupTimer1();

  Serial.println("Timer Initialized");

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found!");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);

  Serial.println("Sensors initialized successfully!");
}

void loop() {
  long irValue = particleSensor.getIR();

  if (irValue > 50000) {
    if (checkForBeat(irValue)) {
      long delta = millis() - lastBeat;
      lastBeat = millis();
      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;

        beatAvg = 0;
        for (byte x = 0; x < RATE_SIZE; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    } else {
      long timeSinceLastBeat = millis() - lastBeat;
      if (timeSinceLastBeat > 2000) {
        beatsPerMinute = 0;
        beatAvg = 0;
      }
    }
  } else {
    beatsPerMinute = 0;
    beatAvg = 0;
  }

  if (timerFlag) {
    timerFlag = false;

    int mq4_value = analogRead(MQ4_PIN);
    int methane_concentration = map(mq4_value, 0, 1023, 0, 1000);

    int mq7_value = analogRead(MQ7_PIN);
    int co_concentration = map(mq7_value, 0, 1023, 0, 1000);

    Serial.println("Timer Interrupt Triggered");

    Serial.print("Methane (MQ-4): ");
    Serial.print(methane_concentration);
    Serial.println(" ppm");

    Serial.print("Carbon Monoxide (MQ-7): ");
    Serial.print(co_concentration);
    Serial.println(" ppm");

    HC12.print(ID);
    HC12.print(",");
    HC12.print(methane_concentration);
    HC12.print(",");
    HC12.print(co_concentration);
    HC12.print(",");
    HC12.print(beatAvg);
    HC12.println("");
    delay(50);
    Serial.print("Avg BPM=");
    Serial.println(beatAvg);
    Serial.println("----------------------");

    if (methane_concentration > METHANE_THRESHOLD) {
      Serial.println("🔥 ALERT: Methane Gas Detected!");
      HC12.print(ID);
      HC12.println("1");
      digitalWrite(BUZZER, HIGH);
    } else if (co_concentration > CO_THRESHOLD) {
      Serial.println("⚠️ ALERT: Carbon Monoxide Detected!");
      HC12.print(ID);
      HC12.println("2");
      digitalWrite(BUZZER, HIGH);
    } else if (beatAvg != 0 && (beatAvg > BPM_HIGH || beatAvg < BPM_LOW)) {
      Serial.println("🚨 ALERT: Abnormal Heart Rate!");
      HC12.print(ID);
      HC12.println("3");
      digitalWrite(BUZZER, HIGH);
    } else {
      digitalWrite(BUZZER, LOW);
    }
  }
}
