#include <SoftwareSerial.h>

SoftwareSerial HC12(D7, D8); //D7- HC-12 TX Pin,D8- HC-12 RX Pin

void setup() {
  Serial.begin(9600);             // Serial port to computer
  HC12.begin(9600);               // Serial port to HC12
  pinMode(2, OUTPUT);
}

void loop() {
  if (HC12.available()) {
    char receivedChar = HC12.read();  // Read the byte
    Serial.print("Received: ");
    Serial.println(receivedChar);

    if (receivedChar == '1') {
      digitalWrite(2, HIGH);  // Turn ON LED or Buzzer
    }
    if (receivedChar == '0') {
      digitalWrite(2, LOW);  // Turn OFF LED or Buzzer
    }
  }
}
