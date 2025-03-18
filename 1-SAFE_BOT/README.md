# SAFE-BOT

- device “SAFEBOT” designed to continuously monitor the vital signs and toxic gases to sanitation workers as they navigate the challenges of sewage cleaning. Our device incorporates cutting-edge technology, including a Carbon Monoxide, Hydrogen sulphide sensors and one Heart Beat sensor to accurately track the worker's pulse rate.

- SAFEBOT is to provide proactive alerts in their own regional language (Ex. Tamil) to both the worker wearing it and an external monitoring unit. These alerts are triggered whenever any of the monitored parameters deviate from the established safe ranges.

## Transmitter and Receiver

- Transmitter side detects the gas sensor values and heart rate of the workers. if the hazardous found, it sends message to the receiver side.

- Receiver side send the data to the cloud for future references.

### march 18

converted ESP32 to Arduino nano in the transmitter side.

## References

- [max30102](https://circuitdigest.com/microcontroller-projects/how-max30102-pulse-oximeter-and-heart-rate-sensor-works-and-how-to-interface-with-arduino)
