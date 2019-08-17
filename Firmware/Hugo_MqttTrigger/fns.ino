void toggleLed(int blinkSpeed) {
  digitalWrite(5, ledState);
  ledState = !ledState;

  ticker.attach_ms(blinkSpeed, toggleLed, blinkSpeed);
}

void startBlinking(int blinkingSpeed) {
  ticker.attach_ms(blinkingSpeed, toggleLed, blinkingSpeed);
}

void stopBlinking() {
  ticker.detach();
  digitalWrite(5, LOW);
}

void goToSleep() {
  Serial.println("going to sleep");

  /*
      This should force all buttons to discharge
      and allow for faster response time
  */
  /*
    pinMode(button1_pin, OUTPUT);
    pinMode(button2_pin, OUTPUT);
    pinMode(button3_pin, OUTPUT);
    pinMode(button4_pin, OUTPUT);
    digitalWrite(button1_pin, LOW);
    digitalWrite(button2_pin, LOW);
    digitalWrite(button3_pin, LOW);
    digitalWrite(button4_pin, LOW);
  */

  yield();
  delay(5);
  ESP.deepSleep(0);
  yield();
}

String macToStr(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

String macLastThreeSegments(const uint8_t* mac) {
  String result;
  for (int i = 2; i < 6; ++i) {
    result += String(mac[i], 16);
  }
  return result;
}

void publishBatteryLevel() {
  client.publish(String("hugo" + macLastThreeSegments(mac) + "/battery").c_str(), String(batteryPercentage()).c_str());
  Serial.print("Battery: ");
  Serial.println(batteryPercentage());
}

void mqtt_connect() {
  //while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(String("Hugo_" + macLastThreeSegments(mac)).c_str())/*, "mqtt_user", "mqtt_pass"*/) {
      Serial.println("MQTT connected.");
    } else {
      Serial.print("MQTT connection attempt failed, rc=");
      Serial.println(client.state());
      goToSleep();
    }
  //}
}

int readButtons() {
  if (digitalRead(button1_pin) == HIGH && digitalRead(button2_pin) == HIGH) {
    return 5;
  }
  if (digitalRead(button2_pin) == HIGH && digitalRead(button3_pin) == HIGH) {
    return 6;
  }
  if (digitalRead(button3_pin) == HIGH && digitalRead(button4_pin) == HIGH) {
    return 7;
  }
  if (digitalRead(button1_pin) == HIGH) {
    return 1;
  } else if (digitalRead(button2_pin) == HIGH) {
    return 2;
  } else if (digitalRead(button3_pin) == HIGH) {
    return 3;
  } else if (digitalRead(button4_pin) == HIGH) {
    return 4;
  }
  return 0;
}

/* Read analog input for battery measurement */
int ReadAIN()
{
  int Read = analogRead(A0);
  int num = 10;

  for (int i = 0 ; i < num ; i++)
  {
    int newRead = analogRead(A0);
    if (newRead > Read)
    {
      Read = newRead;
    }
    delay(1);
  }
  return (Read);
}

/* Battery percentage estimation, this is not very accurate but close enough */
uint8_t batteryPercentage() {
  int analogValue = ReadAIN();
  if (analogValue > 1000) return 200; // CHARGING
  if (analogValue > 960) return 100;
  if (analogValue > 940) return 90;
  if (analogValue > 931) return 80;
  if (analogValue > 922) return 70;
  if (analogValue > 913) return 60; // 3.8v ... 920
  if (analogValue > 904) return 50;
  if (analogValue > 895) return 40;
  if (analogValue > 886) return 30;
  if (analogValue > 877) return 20; // 3.65v ... 880
  if (analogValue > 868) return 10;
  return 0;
}

bool readConfig() {
  File stateFile = SPIFFS.open("/config.json", "r");
  if (!stateFile) {
    Serial.println("Failed to read config file... first run?");
    Serial.println("Creating file and going to sleep. Try again!");
    json["ssid"] = json["pass"] = json["ip"] = json["gw"] = json["sn"] = json["broker"] = json["port"] = json["b1t"] = json["b2t"] = json["b3t"] = json["b4t"] = json["b5t"] = json["b6t"] = json["b7t"] = json["b1p"] = json["b2p"] = json["b3p"] = json["b4p"] = json["b5p"] = json["b6p"] = json["b7p"] = "";
    saveConfig();
    goToSleep();
    return false;
  }
  DeserializationError error = deserializeJson(json, stateFile.readString());
  stateFile.close();
  return true;
}

bool saveConfig() {
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  serializeJson(json, configFile);
  //serializeJson(json, Serial);
  configFile.close();
  return true;
}
