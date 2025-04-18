#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Create your custom WiFi network
const char* ssid = "MyHealth";
const char* password = "123D56HI";  // Must be at least 8 characters

// Create a web server on port 80
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  // Set ESP8266 as Access Point
  WiFi.softAP(ssid, password);

  Serial.println("Access Point Started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP()); // Typically 192.168.4.1

  // Define a simple webpage
  server.on("/", []() {
    server.send(200, "text/html", "<h1>Hello from ESP8266 AP!</h1>");
  });

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}
