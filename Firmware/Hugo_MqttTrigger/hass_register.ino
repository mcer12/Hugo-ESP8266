
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

void sendConfig(StaticJsonDocument<512>& payload, String configTopic) {
  char output[512];
  serializeJson(payload, output);
  Serial.println(output);
  if (client.publish(configTopic.c_str(), output)) {
    Serial.print("Discovery data sent.");
  } else {
    Serial.print("Failed to send discovery data, error = ");
    Serial.println(client.state());
  }
  payload.clear();
}

void doHassRegister() {

  if (deviceMode != HASS_REGISTER_MODE) return;

  Serial.println("Attemting to send discovery data...");
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
  sendConfig(payload, configTopic);

  client.loop();

  Serial.println("Done. Attemting to send battery discovery data...");

  // And battery...
  configTopic = "homeassistant/sensor/hugo_" + macLastThreeSegments(mac) + "/battery/config";
  stateTopic = "homeassistant/sensor/hugo_" + macLastThreeSegments(mac) + "/battery";

  payload["uniq_id"] = "hugo_" + macLastThreeSegments(mac) + "_battery";
  payload["name"] = "Hugo " + macLastThreeSegments(mac) + " - Battery";
  payload["stat_t"] = stateTopic;
  payload["ic"] = "mdi:battery-outline";
  payload["dev_cla"] = "battery";
  payload["unit_of_meas"] = "%";
  //payload["val_tpl"] = "{% if value > 100 %}999{% else %}{{value}}{% endif %}";
  device = payload.createNestedObject("device");
  device["ids"] = "hugo" + macLastThreeSegments(mac);
  device["name"] = "Hugo - WiFi remote";
  device["mf"] = "https://github.com/mcer12/Hugo-ESP8266";
  device["mdl"] = "Hugo-ESP8266";
  device["sw"] = FW_VERSION;
  sendConfig(payload, configTopic);

  client.loop();
  client.disconnect();

  Serial.println("Hugo should now be discovered by Home Assistant. Use following topic to update values:");
  Serial.println(stateTopic);

  digitalWrite(5, HIGH);
  delay(100);
  digitalWrite(5, LOW);

  goToSleep();
}
