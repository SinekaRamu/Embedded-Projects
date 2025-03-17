#include <SoftwareSerial.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define Methane D0   //GPIO 16
#define CO D6        //GPIO 12
#define HeartRate D5 //GPIO 14

SoftwareSerial HC12(D7, D0); // D7(GPIO 13)- HC-12 TX Pin,D8(GPIO 15)- HC-12 RX Pin
const char* ID = "S";

const char* ssid = "Airtel_bala_4993";
const char* password = "air70386";
String serverURL = "https://webroid.in/demo/idhayaattendance/register.php";

WiFiServer server(80);
unsigned long lastDataTime = 0; // Timer to track last received data
const unsigned long dataTimeout = 3000; // Timeout duration in milliseconds (5 seconds)

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

  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  SAFE-BOT");
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
      lcd.print("CH4:" + methaneStr);
      lcd.setCursor(0, 1);
      lcd.print("CO:" + coStr);

      // Optionally, print heart rate on the next line if you have a larger LCD
      lcd.setCursor(9, 1);
      lcd.print(beatAvgStr==" 0"? " HR: NO":(" HR:"+beatAvgStr));
      postData(methaneStr, coStr, beatAvgStr);
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

}

bool postData(String methane, String co, String beatAvg) {
    client.setInsecure();

    http.begin(client, attendanceURL);  
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "meth=" + String(methane)+"co="+String(co)+"beatAvg"+"";
    int httpCode = http.POST(postData);
    String response = http.getString();

    Serial.println("HTTP Code: " + String(httpCode));
    Serial.println("Server Response: " + response);

    // **Check for Connection Failure**
    if (httpCode == -1) {
        Serial.println("⚠️ Connection failed!");
        DisplayMessage("Conn Failed!", "Retry");
        http.end();
        return false;
    }

    // **Check if the Server Responded**
    if (httpCode != 200) {
        Serial.println("⚠️ Server Error: " + String(httpCode));
        DisplayMessage("Server Error", String(httpCode));
        http.end();
        return false;
    }

    // **Parse JSON Response**
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        Serial.print("⚠️ JSON Parsing Error: ");
        Serial.println(error.c_str());
        DisplayMessage("JSON Error", "Invalid Data");
        http.end();
        return false;
    }

    // **Extract JSON Values**
    String status = doc["status"].as<String>();
    String message = doc["message"].as<String>();

    // **Display Message on OLED**
    DisplayMessage(status, message);

    http.end();  // **Close HTTP Connection**
    
    // **Return false if an error occurred**
    return (status != "error");
}