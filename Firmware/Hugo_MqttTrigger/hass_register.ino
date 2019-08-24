
void toggleHassRegister() {
  if (digitalRead(button2_pin) == HIGH && digitalRead(button4_pin) == HIGH) {
    int i = 0;
    while (digitalRead(button2_pin) == HIGH && digitalRead(button4_pin) == HIGH && i < 200) {
      delay(10);

      if (i > 100) {
        deviceMode = HASS_REGISTER_MODE;
        configTimer = millis(); // start counter
        return;
      }
      i++;
    }
  }
}

void sendConfigPart(StaticJsonDocument<512>& payload, String configTopic, int currentPartNo, int partsCount) {
  char output[512];
  serializeJson(payload, output);
  Serial.println(output);
  if (client.publish(configTopic.c_str(), output)) {
    Serial.print("Registration ");
    Serial.print(currentPartNo);
    Serial.print(" of ");
    Serial.print(partsCount);
    Serial.println(" done.");
  } else {
    Serial.print("Registration ");
    Serial.print(currentPartNo);
    Serial.print(" of ");
    Serial.print(partsCount);
    Serial.print(" failed, json too large? Error = ");
    Serial.println(client.state());
  }
  payload.clear();
}

void doHassRegister() {

  if (deviceMode != HASS_REGISTER_MODE) return;

  Serial.println("Registering device");
  StaticJsonDocument<512> payload;
  size_t payloadSize;
  String configTopic = "homeassistant/sensor/hugo_" + macLastThreeSegments(mac) + "/config";
  String stateTopic = "homeassistant/sensor/hugo_" + macLastThreeSegments(mac) + "/state";

  payload["unique_id"] = "hugo_" + macLastThreeSegments(mac);
  payload["name"] = "Hugo " + macLastThreeSegments(mac);
  payload["stat_t"] = stateTopic;
  payload["ic"] = "mdi:remote";
  payload["exp_aft"] = "1";
  JsonObject device = payload.createNestedObject("device");
  device["ids"] = "hugo" + macLastThreeSegments(mac);
  device["name"] = "Hugo - WiFi remote";
  device["mf"] = "https://github.com/mcer12/Hugo-ESP8266";
  device["mdl"] = "Hugo-ESP8266";
  device["sw"] = FW_VERSION;
  sendConfigPart(payload, configTopic, 5, 5);

  client.loop();
  client.disconnect();

  Serial.println("Registration completed, Hugo should now be discovered by Home Assistant. Use following topic to update values:");
  Serial.println(stateTopic);

  digitalWrite(5, HIGH);
  delay(100);
  digitalWrite(5, LOW);

  goToSleep();
}
