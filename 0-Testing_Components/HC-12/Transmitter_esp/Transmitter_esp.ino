#include <SoftwareSerial.h>

SoftwareSerial HC12(18, 19); // HC-12 TX Pin, HC-12 RX Pin

void setup()
{
    Serial.begin(9600); // Serial port to computer
    HC12.begin(9600);   // Serial port to HC12
    Serial.print("ready:)");
}

void loop()
{
    while (HC12.available())
    {                              // If HC-12 has data
        Serial.write(HC12.read()); // Send the data to Serial monitor
    }
    // while (Serial.available())
    // {                              // If Serial monitor has data
    //     char val = Serial.read();
    //     HC12.write(val); // Send that data to HC-12
    //     Serial.println(val);
    // }
    for(int i=0; i<2; i++){
      HC12.write('1');
      Serial.println('1');
      delay(2000);
      HC12.write('0');
      Serial.println('2');
      delay(2000);
    }
    
}