
/*
 *  ESP8266 JSON Decode of NEP server response
 *  Knightwolf
 */
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h> // Display ssh1106
#include <SPI.h>
#include <Wire.h>
#include <Fonts/FreeSans9pt7b.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define i2c_Address 0x3c 
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1  
int SCLpin = D1;
int SDApin = D2;

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


const char* wifiName = "ssd";
const char* wifiPass = "pasword";
char daysOfTheWeek[7][12] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
//Web Server address to read/write from 
const char *host = "https://user.nepviewer.com/pv_monitor/proxy/status/xxxxxxxx/0/2/"; //https://user.nepviewer.com/pv_monitor/proxy/status/xxxxxx/0/2/


void setup() {
  
  Serial.begin(115200);
  delay(10);
  Wire.begin(SCLpin,SDApin);           // Display Pin
  display.begin(i2c_Address, true);    // Display I2C
  
  display.setTextSize(1);
  display.clearDisplay();
  display.setCursor(10, 20);
  display.setTextColor(SH110X_WHITE);
  display.println("WIFI Start");
  Serial.print("Connecting to ");
  Serial.println(wifiName);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiName, wifiPass);

 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());   //You can get IP address assigned to ESP
  Serial.println(WiFi.RSSI());
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(7200);

}

void loop() {
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  
  Serial.print("Request Link:");
  Serial.println(host);
  
  https.begin(*client, host); //Specify request destination
     
  
  int httpCode = https.GET();            //Send the request
  String input = https.getString();      //Get the response payload from server

  Serial.print("Response Code:");       //200 is OK
  Serial.println(httpCode);             //Print HTTP return code

  Serial.print("Returned data from Server:");
  Serial.println(input);                 //Print request response payload

StaticJsonDocument<512> doc; // Static Ram

DeserializationError error = deserializeJson(doc, input);

if (error) {
  Serial.print(F("deserializeJson() failed: "));
  Serial.println(error.f_str());
  return;
}

//const char* addr = doc["addr"]; // "0"
//Serial.println(addr);
//const char* LastUpDate_Stamp = doc["LastUpDate_Stamp"]; // "1690995685"
//Serial.println(LastUpDate_Stamp);
const char* LastUpDate = doc["LastUpDate"]; // "2023-08-02 17:01"
Serial.println(LastUpDate);
//const char* localtime = doc["localtime"]; // "2023-08-02 23:02:52"
//Serial.println(localtime);
int now = doc["now"]; // 55
Serial.print(now);
Serial.println(" W");
int today = doc["today"]; // 958
float today2 = today;
Serial.print(today2/1000, 2);
Serial.println(" kW Heute");
//int total_status = doc["total_status"]; // 5457
//Serial.println(total_status);
//int total_year = doc["total_year"]; // 0
//Serial.println(total_year);
//int total = doc["total"]; // 5457
//Serial.println(total);
//int co2 = doc["co2"]; // 6
//const char* status = doc["status"]; // "0000"

  timeClient.update();         // Liest die aktuelle Zeit
 
Serial.println(timeClient.getEpochTime());
Serial.print(daysOfTheWeek[timeClient.getDay()]);
Serial.print(", ");
Serial.print(timeClient.getHours());
Serial.print(":");
Serial.print(timeClient.getMinutes());

Serial.printf(" \t %s %01d:%02d Uhr\n", daysOfTheWeek[timeClient.getDay()], timeClient.getHours(), timeClient.getMinutes());
      display.clearDisplay();
      display.setTextSize(2);
      //display.setFont(&FreeSans9pt7b);      //FreeSans9pt7b.h
      display.setTextColor(SH110X_WHITE);
      display.setCursor(5, 5);
      display.print(now);
      display.print(" W Now");
      display.setCursor(5, 30);
      display.print(today2/1000);
      display.print("KW Day");

      display.setTextSize(1);
      display.drawFastHLine(0, 52, 128, SH110X_WHITE); //Linie

      display.setCursor(4, 55);
       display.printf("\t %s %01d:%02d Uhr\n", daysOfTheWeek[timeClient.getDay()], timeClient.getHours(), timeClient.getMinutes());

      display.display();
    
  https.end();  //Close connection
  
  delay(60000);  //GET Data at every 5 seconds
}
