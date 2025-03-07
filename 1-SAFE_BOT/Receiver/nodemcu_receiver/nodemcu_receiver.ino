#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

SoftwareSerial HC12(D7, D8); // D7- HC-12 TX Pin,D8- HC-12 RX Pin
const char* ID = "S";

const char* ssid = "Airtel_bala_4993";
const char* password = "air70386";

WiFiServer server(80);

void setup()
{
  Serial.begin(9600); // Serial port to computer
  HC12.begin(9600);   // Serial port to HC12

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D4, OUTPUT);

  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D4, HIGH);
}

void loop() {
  if (HC12.available()) {
    String receivedMessage = HC12.readStringUntil('\n');
    receivedMessage.trim(); // Remove any white spaces or newline characters

    if (receivedMessage.startsWith(ID)) {
      char command = receivedMessage.charAt(1);
      Serial.println(command);

      digitalWrite(D1, HIGH);
      digitalWrite(D2, HIGH);
      digitalWrite(D4, HIGH);

      if (command == '1') {
        Serial.println("Methene detected");
        digitalWrite(D1, LOW);
      } else if (command == '2') {
        Serial.println("CO detected");
        digitalWrite(D2, LOW);
      } else if (command == '3') {
        Serial.println("Heart pulse");
        digitalWrite(D4, LOW);
      }
    }
  }
}
