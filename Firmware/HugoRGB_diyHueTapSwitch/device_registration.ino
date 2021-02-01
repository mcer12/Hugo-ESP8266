void toggleRegisterRequest() {
  if (digitalRead(button2_pin) == HIGH && digitalRead(button4_pin) == HIGH) {
    int i = 0;
    while (digitalRead(button2_pin) == HIGH && digitalRead(button4_pin) == HIGH && i < 200) {
      delay(10);

      if (i > 100) {
        registerNewRemote();
        blinkLedRGB(1000, 0, 10, 0); // blink green, 1s
        goToSleep();
        return;
      }
      i++;
    }
  }
}

bool registerNewRemote() {
  const char* bridgeIp = json["bridge"].as<const char*>();
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
