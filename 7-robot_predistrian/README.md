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

## issues

### issue 1

- if ultrasonic sensor detects any object it should stop working the robot, but it still robot run for 1 sec

- Solution: using interrupt with one ultrasonic sensor

### issue 2

- Resume from the last command crashes with error `Guru Meditation Error: Core  1 panic'ed (StoreProhibited). Exception was unhandled.
` 

The crash is likely happening when your code tries to resume a previous movement command after obstacle detection, but it accidentally re-executes 'x' (stop command) or improperly tries to resume motion without proper motor setup or display handling.
