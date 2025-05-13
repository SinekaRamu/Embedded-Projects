# Vibration-Based Drone Takeoff System

1. Introduction

Earthquakes cause massive destruction and disrupt emergency response operations. A rapid and autonomous system for deploying drones in response to seismic activity can significantly aid in search and rescue missions. This project presents a vibration-based drone takeoff system that detects earthquake tremors using an MPU6050 sensor and autonomously initiates a drone takeoff sequence for aerial surveillance and emergency response.

## 2. System Overview

The system consists of an MPU6050 accelerometer and gyroscope to detect vibrations and determine earthquake intensity. When vibrations exceed a predefined threshold, the system activates an HC-12 wireless module to send a takeoff signal to a drone, which is then stabilized using a PID controller.

## 3. Components Used in Drone

- Microcontroller: Arduino Uno

- IMU Sensor: MPU6050 (Accelerometer + Gyroscope)

- Wireless Module: HC-12

- Quadcopter frame

- 4 x Brushless DC motors with ESCs

- Propellers

- Power Source: Li-Po battery

## 4. Components used in Transmitter side

- Arduino Nano
- Vibration sensor
- HC-12 module
- LCD Display
- Battery 9V

## 5. Working Principle

- Vibration Detection:

  - The vibration sensor continuously vibration for a project purpose the small vibration for 2 second is taken as signal, sent to receiver.

- Wireless Communication:

  - The Arduino Nano transmits a signal via HC-12 to the drone controller.

  - The drone receives and verifies the command before takeoff.

- Drone Takeoff and Stabilization: this part is yet to do :(

  - The drone stabilizes using the PID controller for pitch and roll adjustments.

  - Motor speed is adjusted based on gyro readings to maintain balance.

  - If excessive tilt is detected, corrections are applied automatically.

## 6. Expected Outcomes

- Autonomous takeoff of drones upon earthquake detection.

- Stable flight using PID-based adjustments.

- Real-time transmission of earthquake alerts.

## 7. Future Enhancements

- Integration of GPS for location tracking.

- Addition of camera modules for aerial footage.

- Implementation of ML-based earthquake classification.

## 8. Conclusion

This system provides an efficient, real-time solution for deploying drones in the event of earthquakes. By leveraging vibration detection, wireless communication, and stabilization techniques, this project can improve emergency response and disaster management efforts.

## Important

- Constant Current: 30A (Max 40A<10s). BEC: 5V 3A. Input Voltage: 11.1 ~ 11.7 V. Li-Po battery: 2S ~ 3S
- if the motor casing and one of the three wire is shorted, then the motor is damaged. <b>remove the motor</b> or it will damage the ESC.
