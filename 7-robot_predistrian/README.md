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

## Raspberry pi setup

- `sudo apt-get update` updating raspberry pi software

  - current python version -- 3.11.2

- To download the dependencies for opencv, I used below command.

  - `sudo apt-get install libhdf5-dev -y && sudo apt-get install libhdf5-serial-dev -y && sudo apt-get install libatlas-base-dev -y && sudo apt-get install libjasper-dev -y && sudo apt-get install libqtgui4 -y && sudo apt-get install libqt4-test -y`

- Now let's install Opencv for python, version is what I'm currently using

  - `pip3 install opencv-contrib-python==4.11.0.86`
  - OpenCV version - 4.11.0

- wget https://raw.githubusercontent.com/shantnu/Webcam-Face-Detect/master/haarcascade_frontalface_default.xml - it's for face detection and we're not gonna use it!

- Now We gonna download tensorflow `pip3 install tensorflow`

- Download MobileNet SSD Model and Prototxt

```
mkdir mobilenet-ssd
cd mobilenet-ssd

//Download the Caffe model and config:

wget https://huggingface.co/spaces/Imran606/cds/resolve/main/MobileNetSSD_deploy.caffemodel

wget https://gist.githubusercontent.com/mm-aditya/797a3e7ee041ef88cd4d9e293eaacf9f/raw/3d2765b625f1b090669a05d0b3e79b2907677e86/MobileNetSSD_deploy.prototxt

```
- To check the file use command

```
pwd - shows the current folder path

ls -lh - displays the hidden files

```
- Checking pi camera using the command `libcamera-still -o test.jpg`

## issues

### issue 1

- if ultrasonic sensor detects any object it should stop working the robot, but it still robot run for 1 sec

- Solution: using interrupt with one ultrasonic sensor

### issue 2

- Resume from the last command crashes with error `Guru Meditation Error: Core  1 panic'ed (StoreProhibited). Exception was unhandled.
`

The crash is likely happening when your code tries to resume a previous movement command after obstacle detection, but it accidentally re-executes 'x' (stop command) or improperly tries to resume motion without proper motor setup or display handling.

- solution: display and motor handled separately and added condition for x to stop
