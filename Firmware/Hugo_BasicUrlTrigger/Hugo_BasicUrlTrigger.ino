/*
  Hugo ESP8266
  basic firmware

  This is the original firmware bundled with the remote.

  For more information and help, head to Hugo's github repository:
  https://github.com/mcer12/Hugo-ESP8266

  Do you find Hugo great? Get yourself another one on Tindie:
  https://www.tindie.com/products/nicethings/hugo-esp8266-4-button-wifi-remote/

  Credits to Marius Motea for his great project. His project was the reason to design
  the remote in the first place and firmware sketch was initially based on it.
  https://github.com/diyhue/diyHue

  ***

  MIT License

  Copyright (c) 2019 Martin Cerny

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <FS.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

#include <ArduinoOTA.h>
#include <Ticker.h>

#define OTA_NAME "Hugo"
#define AP_NAME "HugoConfig"
#define button1_pin 14
#define button2_pin 4
#define button3_pin 12
#define button4_pin 13
#define OTA_BLINK_SPEED 100
#define OTA_TIMEOUT 300000 // 5 minutes
#define CONFIG_BLINK_SPEED 500
#define CONFIG_TIMEOUT 300000 // 5 minutes
// DO NOT CHANGE DEFINES BELOW
#define NORMAL_MODE 0
#define OTA_MODE 1
#define CONFIG_MODE 2

uint8_t deviceMode = NORMAL_MODE;

int button;

int buttonTreshold = 2000;

// BUTTONS TOGGLES
bool button1toggled = false, button2toggled = false, button3toggled = false, button4toggled = false;
bool otaModeStarted = false;
volatile bool ledState = false;

// TIMERS
unsigned long otaMillis, sleepMillis, ledMillis, configTimer, otaTimer;

int counter;
byte mac[6];

Ticker ticker;
ESP8266WebServer server(80);

DynamicJsonDocument json(1024); // config buffer

void setup() {
  Serial.begin(115200);
  Serial.println("");

  pinMode(16, OUTPUT);
  pinMode(button1_pin, INPUT);
  pinMode(button2_pin, INPUT);
  pinMode(button3_pin, INPUT);
  pinMode(button4_pin, INPUT);
  pinMode(5, OUTPUT);

  digitalWrite(16, LOW);
  digitalWrite(5, LOW);

  delay(10); // This small delay is required for correct button detection

  button = readButtons();
  Serial.println(button);

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
  }

  readConfig();

  const char* ssid = json["ssid"].as<const char*>();
  const char* pass = json["pass"].as<const char*>();
  const char* ip = json["ip"].as<const char*>();
  const char* gw = json["gw"].as<const char*>();
  const char* sn = json["sn"].as<const char*>();

  if (ssid[0] != '\0' && pass[0] != '\0') {
    WiFi.mode(WIFI_STA);

    if (ip[0] != '\0' && gw[0] != '\0' && sn[0] != '\0') {
      IPAddress ip_address, gateway_ip, subnet_mask;
      if (!ip_address.fromString(ip) || !gateway_ip.fromString(gw) || !subnet_mask.fromString(sn)) {
        Serial.println("Error setting up static IP, using auto IP instead. Check your configuration.");
      } else {
        WiFi.config(ip_address, gateway_ip, subnet_mask);
      }
    }

    //serializeJson(json, Serial);

    WiFi.begin(ssid, pass);

    int iterator = 0;
    while (WiFi.status() != WL_CONNECTED) {
      iterator++;
      if (iterator > 400) { // 4s timeout
        deviceMode = CONFIG_MODE;
        Serial.print("Failed to connect to: ");
        Serial.println(ssid);
        break;
      }
      delay(10);
    }
    /*
      Serial.println("Wifi connected...");
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());
      Serial.print("Mac address: ");
      Serial.println(WiFi.macAddress());
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    */
    WiFi.macAddress(mac);
  } else {
    deviceMode = CONFIG_MODE;
    Serial.println("No credentials set, go to config mode");
    //startConfigPortal();
    //goToSleep();
  }

  rst_info *rinfo;
  rinfo = ESP.getResetInfoPtr();

  String ota_name = "Hugo_" + macLastThreeSegments(mac);
  ArduinoOTA.setHostname(ota_name.c_str());
  ArduinoOTA.begin();

  ArduinoOTA.onStart([]() {
    Serial.println("OTA UPLOAD STARTED...");
    stopBlinking();
    digitalWrite(5, HIGH);
  });

}

void loop() {

  if (deviceMode == OTA_MODE) {
    Serial.println("WAITING FOR OTA UPDATE...");
    startOTA();
    Serial.println("RETURNING TO NORMAL MODE...");
    return;
  }

  if (deviceMode == CONFIG_MODE) {
    Serial.println("STARTING CONFIG ACCESS POINT, PRESS ANY BUTTON TO EXIT...");
    startConfigPortal();
    Serial.println("RETURNING TO NORMAL MODE...");
    return;
  }

  toggleOTAMode();

  toggleConfigMode();

  if (deviceMode != NORMAL_MODE) return;

  if (button == 1) {
    Serial.println("B1");
    sendHttpRequest(json["b1"].as<String>());
  }
  else if (button == 2) {
    Serial.println("B2");
    sendHttpRequest(json["b2"].as<String>());
  }
  else if (button == 3) {
    Serial.println("B3");
    sendHttpRequest(json["b3"].as<String>());
  }
  else if (button == 4) {
    Serial.println("B4");
    sendHttpRequest(json["b4"].as<String>());
  }
  else if (button == 5) {
    Serial.println("B5 (B1+B2 combo)");
    sendHttpRequest(json["b5"].as<String>());
  }
  else if (button == 6) {
    Serial.println("B6 (B2+B3 combo)");
    sendHttpRequest(json["b6"].as<String>());
  }
  else if (button == 7) {
    Serial.println("B7 (B3+B4 combo)");
    sendHttpRequest(json["b7"].as<String>());
  }
  digitalWrite(5, HIGH);
  delay(20);
  digitalWrite(5, LOW);

  //if (millis() - sleepMillis >= sleepDelay) {
  goToSleep();
  //}

}
