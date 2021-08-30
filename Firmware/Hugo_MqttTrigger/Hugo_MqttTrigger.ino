/*
  Hugo ESP8266
  MQTT firmware

  For more information and help, head to Hugo's github repository:
  https://github.com/mcer12/Hugo-ESP8266

  Do you find Hugo great? Get yourself another one on Tindie:
  https://www.tindie.com/products/nicethings/hugo-esp8266-4-button-wifi-remote/

  Credits to gon for providing his modified version of BasicUrlTrigger firmware
  which was used as a starting point for this firmware.

  Credits to Marius Motea for his great project. His project was the reason to design
  the remote in the first place and firmware sketch was initially based on it.
  https://github.com/diyhue/diyHue

  May contain code lines from the PubSubClient examples

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
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <Ticker.h>

#define SKETCH "MqttTrigger"
#define OTA_NAME "Hugo_" // Last 6 MAC address characters will be appended at the end of the OTA name, "Hugo_XXXXXX" by default
#define AP_NAME "Hugo_" // Last 6 MAC address characters will be appended at the end of the AP name, "Hugo_XXXXXX" by default
#define FW_VERSION "1.4.4"
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
#define CONFIG_MODE_LOCAL 3
#define HASS_REGISTER_MODE 4
#if MQTT_MAX_PACKET_SIZE < 512  // If the max message size is too small, throw an error at compile time. See PubSubClient.cpp line 359
#error "MQTT_MAX_PACKET_SIZE is too small in libraries/PubSubClient/src/PubSubClient.h, increase it to 512"
#endif

uint8_t deviceMode = NORMAL_MODE;

int button;
uint8_t batteryPercentage;

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
WiFiClient espClient;
PubSubClient client(espClient);

DynamicJsonDocument json(2048); // config buffer

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

  delay(20); // This small delay is required for correct button detection

  batteryPercentage = getBatteryPercentage();
  button = readButtons();

  Serial.print("FW: ");
  Serial.println(SKETCH);
  Serial.print("Button: ");
  Serial.println(button);
  Serial.print("Battery percentage: ");
  Serial.println(batteryPercentage);


  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
  }

  WiFi.macAddress(mac);
  readConfig();

  const char* ssid = json["ssid"].as<const char*>();
  const char* pass = json["pass"].as<const char*>();
  const char* ip = json["ip"].as<const char*>();
  const char* gw = json["gw"].as<const char*>();
  const char* sn = json["sn"].as<const char*>();

  const char* broker = json["broker"].as<const char*>();
  int port = json["port"].as<int>();

  if (ssid[0] != '\0' && pass[0] != '\0') {
    Serial.println("setting up wifi");
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
      if (iterator > 100) { // 10s timeout
        deviceMode = CONFIG_MODE;
        Serial.print("Failed to connect to: ");
        Serial.println(ssid);
        break;
      }
      delay(100);
    }
    Serial.println("Wifi connected...");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("Mac address: ");
    Serial.println(WiFi.macAddress());
    WiFi.macAddress(mac);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    deviceMode = CONFIG_MODE;
    Serial.println("No credentials set, go to config mode");
  }

  // MQTT SETUP
  if (broker[0] != '\0' && port != 0) {
    client.setServer(broker, port);
  } else {
    deviceMode = CONFIG_MODE;
    Serial.println("Broker address or port is not set, going to config mode.");
  }

} else {
  deviceMode = CONFIG_MODE;
  Serial.println("No credentials set, going to config mode.");
  //startConfigPortal();
  //goToSleep();
}

rst_info *rinfo;
rinfo = ESP.getResetInfoPtr();

setupOTA();

}

void loop() {

  toggleOTAMode();

  toggleConfigMode();

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

  mqtt_connect();

  if (deviceMode == HASS_REGISTER_MODE) {
    Serial.println("STARTING HOME ASSISTANT DISCOVERY...");
    doHassRegister();
    Serial.println("RETURNING TO NORMAL MODE...");
    return;
  }

  toggleHassRegister();

  if (deviceMode == NORMAL_MODE) {

    if (button == 1) {
      Serial.println("B1");
      String b1t = json["b1t"].as<String>();
      String b1p = json["b1p"].as<String>();
      publishButtonData(b1t, b1p);
      blinkLed(20);
    }
    else if (button == 2) {
      Serial.println("B2");
      String b2t = json["b2t"].as<String>();
      String b2p = json["b2p"].as<String>();
      publishButtonData(b2t, b2p);
      blinkLed(20);
    }
    else if (button == 3) {
      Serial.println("B3");
      String b3t = json["b3t"].as<String>();
      String b3p = json["b3p"].as<String>();
      publishButtonData(b3t, b3p);
      blinkLed(20);
    }
    else if (button == 4) {
      Serial.println("B4");
      String b4t = json["b4t"].as<String>();
      String b4p = json["b4p"].as<String>();
      publishButtonData(b4t, b4p);
      blinkLed(20);
    }
    else if (button == 5) {
      Serial.println("B6 (B1+B2 combo)");
      String b5t = json["b5t"].as<String>();
      String b5p = json["b5p"].as<String>();
      publishButtonData(b5t, b5p);
      blinkLed(20);
    }
    else if (button == 6) {
      Serial.println("B6 (B2+B3 combo)");
      String b6t = json["b6t"].as<String>();
      String b6p = json["b6p"].as<String>();
      publishButtonData(b6t, b6p);
      blinkLed(20);
    }
    else if (button == 7) {
      Serial.println("B7 (B3+B4 combo)");
      String b7t = json["b7t"].as<String>();
      String b7p = json["b7p"].as<String>();
      publishButtonData(b7t, b7p);
      blinkLed(20);
    }
  }

  publishBatteryLevel();

  client.loop();
  client.disconnect();

  if (deviceMode == NORMAL_MODE) {

    if (batteryPercentage < 10) {
      delay(500);
      lowBatteryAlert();
    }

    goToSleep();
  }
}
