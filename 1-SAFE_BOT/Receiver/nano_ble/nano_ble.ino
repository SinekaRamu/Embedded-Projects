#include <SoftwareSerial.h>
SoftwareSerial BTSerial(2, 3);  // RX, TX

void setup() {
  Serial.begin(9600);

  BTSerial.begin(9600);  // Default AT mode baud rate for HC-05
}

void loop() {

  if(BTSerial.available()){
    Serial.print(BTSerial.read());
  }
  if(Serial.available()){
    BTSerial.write(Serial.read());
  }

  // if (BTSerial.available()) {
  //   String received = BTSerial.readString();
  //   Serial.println("Message: " + received);
  // }
  
}


