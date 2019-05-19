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

void sendHttpRequest(const char* buttonUrl) {
  if (buttonUrl[0] == '\0') {
    Serial.println("Button URL is not defined. Set it in config portal.");
    return;
  }
  HTTPClient http;
  http.begin(buttonUrl);

  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.print("Successful request to URL: ");
  } else {
    Serial.print("Error connecting to URL: ");
  }

  Serial.println(buttonUrl);
  http.end();   //Close connection
}

int readButtons() {
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

bool readConfig() {
  File stateFile = SPIFFS.open("/config.json", "r");
  if (stateFile) {
    DeserializationError error = deserializeJson(json, stateFile.readString());
    stateFile.close();
    //Serial.println("json:");
    //serializeJson(json, Serial);
    return true;
  } else {
    Serial.println("Failed to read config file... first run?");
    json["ssid"] = json["pass"] = json["ip"] = json["gw"] = json["sn"] = json["b1"] = json["b2"] = json["b3"] = json["b4"] = "";
    return false;
  }
}
