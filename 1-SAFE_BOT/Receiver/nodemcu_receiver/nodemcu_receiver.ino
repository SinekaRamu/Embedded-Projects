#include <SoftwareSerial.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DEVICE_NAME "safe1234"
#define Methane D0   //GPIO 16
#define CO D6        //GPIO 12
#define HeartRate D5 //GPIO 14

SoftwareSerial HC12(D7, D0); // D7(GPIO 13)- HC-12 TX Pin,D8(GPIO 15)- HC-12 RX Pin
const char* ID = "S";

const char* ssid = "gokula"; 
const char* password = "gokul123";
String serverUrl = "https://ediylabs.com/demo/safebot/inser_data.php";

// Store latest sensor values
String lastMethane = "0";
String lastCO = "0";
String lastHeartRate = "0";

WiFiServer server(80);
unsigned long lastDataTime = 0; // Timer to track last received data
const unsigned long dataTimeout = 5000; // Timeout duration in milliseconds (5 seconds)

unsigned long lastPostTime = 0; // Track last post time
const unsigned long postInterval = 30000; // 30 seconds

// LCD Setup (16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClientSecure client;
HTTPClient http;

void setup()
{
  Serial.begin(9600); // Serial port to computer
  HC12.begin(9600);   // Serial port to HC12

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("System Initializing");

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

  pinMode(Methane, OUTPUT);
  pinMode(CO, OUTPUT);
  pinMode(HeartRate, OUTPUT);

  digitalWrite(Methane, HIGH);
  digitalWrite(CO, HIGH);
  digitalWrite(HeartRate, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  SAFE-BOT");
  delay(2000);
}

void loop() {
  bool dataReceived = false; // Flag to detect data availability
  bool methaneDetected = false;
  bool coDetected = false;
  bool heartRateDetected = false;

  if (HC12.available()) {
    String receivedMessage = HC12.readStringUntil('\n');
    receivedMessage.trim(); // Remove any white spaces or newline characters
    
    if (receivedMessage.startsWith("S,")) {
      receivedMessage = receivedMessage.substring(2); // Remove "S," from the string

      Serial.println("Raw Data: " + receivedMessage);
      // Raw Data: 253.00, 34.00, 74

      char* token = strtok((char*)receivedMessage.c_str(), ",");
      int index = 0;
      String methaneStr, coStr, beatAvgStr;

      while (token != NULL) {
        if (index == 0) {
          methaneStr = token;
        } else if (index == 1) {
          coStr = token;
        } else if (index == 2) {
          beatAvgStr = token;
  
        }
        token = strtok(NULL, ",");
        index++;
      }

      // Print on LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CH4: " + methaneStr);
      lcd.setCursor(0, 1);
      lcd.print("CO: " + coStr);

      // Optionally, print heart rate on the next line if you have a larger LCD
      lcd.setCursor(8, 1);
      lcd.print(" HR: "+beatAvgStr);
      
      lastMethane = methaneStr;
      lastCO = coStr;
      lastHeartRate = beatAvgStr;
    }
  
    if (receivedMessage.startsWith(ID)) {
      char command = receivedMessage.charAt(1);
      Serial.println(command);

      if (command == '1') {
        digitalWrite(Methane, LOW);
        methaneDetected = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Methane detected");
        Serial.println("Methane detected");
        delay(2000);
      } else if (command == '2') {
        digitalWrite(CO, LOW);
        coDetected = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("CO detected");
        Serial.println("CO detected");
        delay(2000);
      } else if (command == '3') {
        digitalWrite(HeartRate, LOW);
        heartRateDetected = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("pulse unstable");
        Serial.println("Heart pulse");
        delay(3000);
      }
      lastDataTime = millis(); // Reset timer
      dataReceived = true;     // Data detected
    }
  }

  // Check if timeout expired
  if (!dataReceived && (millis() - lastDataTime > dataTimeout)) {
    if (!methaneDetected) digitalWrite(Methane, HIGH);
    if (!coDetected) digitalWrite(CO, HIGH);
    if (!heartRateDetected) digitalWrite(HeartRate, HIGH);
  }

  // ⏳ **Post data every 30 seconds**
  if (millis() - lastPostTime >= postInterval) {
    lastPostTime = millis(); // Update timer
    postData(lastMethane, lastCO, lastHeartRate); // Send last received values
  }

}

void postData(String methane, String co, String heartRate) {
    client.setInsecure();

    if (WiFi.status() == WL_CONNECTED) {
    // WiFiClient client;
    // HTTPClient http;
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Sensor Data
    String deviceToken = DEVICE_NAME;

    // Format Data
    String postData = "device_token=" + String(DEVICE_NAME) + 
                      "&methane=" + methane + 
                      "&co=" + co + 
                      "&heart_rate=" + heartRate;

    // Send HTTP POST Request
    int httpResponseCode = http.POST(postData);
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);

    // Read Response
    String response = http.getString();
    Serial.println("Server Response: " + response);

    http.end();
  } else {
    Serial.println("WiFi Not Connected!");
  }
}