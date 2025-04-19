# ROBOT WITH PREDISTRIAN DETECTION

Robot will move according to voice send from bluetooth while moving forware if it detects any object then picture is taken using raspberry pi camera and the object is identified using raspberrypi. if the person is detected then message is send to esp32 and audio message will be played along with that.

## Components Required

- ESP32 (bluetooth)
- ultrasonic sensor (2)
- 0.91 inch 128x32 OLED Display
- Raspberry pi 4
- Raspberry pi camera
- Audio output
- L298N Driver
- BO motor (4)
- 4 wheel robot setup
- lithium battery 3 cell
- breadboard

## connection for ESP32

- Ultrasonic sensor

  - trigFront GPIO 22
  - echoFront GPIO 23
  - trigBack GPIO 22
  - echoBack GPIO 23

- Motor Control

  - ENA 16 (RX2) // Left motor speed control (must be PWM capable)
  - ENB 17 (TX2) // Right motor speed control (must be PWM capable)
  - IN1 5 (D5) // Left motor forward
  - IN2 18 (D18) // Left motor backward
  - IN3 19 (D19) // Right motor forward
  - IN4 23 (D15) // Right motor backward

- OLED display
  - Vin - 3.3V
  - GND - Gnd
  - SDA - D21
  - SCL - D22
