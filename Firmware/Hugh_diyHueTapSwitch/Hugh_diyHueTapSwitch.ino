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

#define OTA_NAME "Hugh"
#define AP_NAME "HughConfig"
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
const char* switchType = "ZGPSwitch";

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
  Serial.begin(74880);

  pinMode(16, OUTPUT);
  pinMode(button1_pin, INPUT);
  pinMode(button2_pin, INPUT);
  pinMode(button3_pin, INPUT);
  pinMode(button4_pin, INPUT);
  pinMode(5, OUTPUT);

  digitalWrite(16, LOW);
  digitalWrite(5, LOW);

  button = readButtons();

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
  }

  readConfig();

  const char* ssid = json["ssid"].as<const char*>();
  const char* pass = json["pass"].as<const char*>();
  const char* ip = json["ip"].as<const char*>();
  const char* gw = json["gw"].as<const char*>();
  const char* sn = json["sn"].as<const char*>();
  const char* bridgeIp = json["bridge"].as<const char*>();

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

    serializeJson(json, Serial);

    WiFi.begin(ssid, pass);

    int iterator = 0;
    while (WiFi.status() != WL_CONNECTED) {
      if (iterator > 100) {
        deviceMode = CONFIG_MODE;
        Serial.print("Failed to connect to wifi");
        break;
      }
      delay(5);
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
    //startConfigPortal();
    //goToSleep();
  }

  rst_info *rinfo;
  rinfo = ESP.getResetInfoPtr();

  if ((*rinfo).reason != REASON_DEEP_SLEEP_AWAKE) {

    WiFiClient client;
    client.connect(bridgeIp, 80);

    //register device
    String url = "/switch";
    url += "?devicetype=" + (String)switchType;
    url += "&mac=" + macToStr(mac);

    //###Registering device
    client.connect(bridgeIp, 80);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + bridgeIp + "\r\n" +
                 "Connection: close\r\n\r\n");
  }

  ArduinoOTA.setHostname(OTA_NAME);
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
    sendHttpRequest(34);
  }
  else if (button == 2) {
    Serial.println("B2");
    sendHttpRequest(16);
  }
  else if (button == 3) {
    Serial.println("B3");
    sendHttpRequest(17);
  }
  else if (button == 4) {
    Serial.println("B4");
    sendHttpRequest(18);
  }
  digitalWrite(5, HIGH);
  delay(20);
  digitalWrite(5, LOW);

  //if (millis() - sleepMillis >= sleepDelay) {
  goToSleep();
  //}

}
