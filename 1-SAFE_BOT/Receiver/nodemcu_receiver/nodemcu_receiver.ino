#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

SoftwareSerial HC12(D7, D8); // D7- HC-12 TX Pin,D8- HC-12 RX Pin

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

  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
}

void loop()
{
  if (HC12.available())
  {
    char command = HC12.read(); // Read the byte
    Serial.print("Received: ");
    Serial.println(command); // receivedChar
    if (command == '1')
    {
      Serial.println("Methene detected");
      digitalWrite(3, HIGH);
    }
    if (command == '2')
    {
      Serial.println("CO detected");
      digitalWrite(4, HIGH);
    }
    if (command == '3')
    {
      Serial.println("heart pluse");
      digitalWrite(5, HIGH);
    }
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
  }
}
