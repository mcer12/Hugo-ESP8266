
void startConfigPortal() {
  startBlinking(CONFIG_BLINK_SPEED);
  WiFi.mode(WIFI_AP);
  IPAddress ap_ip(10, 10, 10, 0);
  WiFi.softAPConfig(ap_ip, ap_ip, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_NAME);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("IP address: ");
  Serial.println(ip);
  server.on("/", handleRoot);
  server.begin();
  
  delay(5000);
  while (deviceMode == CONFIG_MODE) { // BLOCKING INFINITE LOOP
    if (digitalRead(button1_pin) == HIGH || digitalRead(button2_pin) == HIGH || digitalRead(button3_pin) == HIGH || digitalRead(button4_pin) == HIGH || millis() - configTimer > CONFIG_TIMEOUT) {
      stopBlinking();
      goToSleep();
      return;
    }
    server.handleClient();
  }
  
}

void toggleConfigMode() {
  if (digitalRead(button1_pin) == HIGH && digitalRead(button2_pin) == HIGH) {
    int i = 0;
    while (digitalRead(button1_pin) == HIGH && digitalRead(button2_pin) == HIGH && i < 200) {
      delay(10);

      if (i > 100) {
        deviceMode = CONFIG_MODE;
        configTimer = millis(); // start counter
        return;
      }
      i++;
    }
  }
}

void handleRoot() {
  if (server.args()) {

    if (server.hasArg("ssid")) {
      json["ssid"] = server.arg("ssid");
    }
    if (server.hasArg("pass")) {
      json["pass"] = server.arg("pass");
    }
    if (server.hasArg("ip")) {
      json["ip"] = server.arg("ip");
    }
    if (server.hasArg("gw")) {
      json["gw"] = server.arg("gw");
    }
    if (server.hasArg("sn")) {
      json["sn"] = server.arg("sn");
    }
    if (server.hasArg("bridge")) {
      json["bridge"] = server.arg("bridge");
    }
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("Failed to open config file for writing");
      return;
    }
    serializeJson(json, configFile);
    //serializeJson(json, Serial);
    configFile.close();
  }

  String html = "<!DOCTYPE html> <html> <head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <title>Hugh Configuration</title> <style> html,body{ margin: 0; padding: 0; font-size: 16px; background: #444444; } body,*{ box-sizing: border-box; font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto, \"Helvetica Neue\", Arial, sans-serif; } a{ color: inherit; text-decoration: underline; } .wrapper{ margin: 50px 0; } .container{ margin: auto; padding: 40px; max-width: 500px; color: #fff; background: #000; box-shadow: 0 0 100px rgba(0,0,0,.5); border-radius: 50px; } .row{ margin-bottom: 15px; } h1{ margin: 0 0 10px 0; font-family: Arial, sans-serif; font-weight: 300; font-size: 2rem; } h1 + p{ margin-bottom: 30px; } h2{ margin: 30px 0 0 0; font-family: Arial, sans-serif; font-weight: 300; font-size: 1.5rem; } p{ font-size: .85rem; margin: 0 0 20px 0; color: rgba(255,255,255,.7); } label{ display: block; width: 100%; margin-bottom: 5px; } input[type=\"text\"], input[type=\"password\"]{ display: inline-block; width: 100%; height: 42px; line-height: 38px; padding: 0 20px; color: #fff; border: 2px solid #666; background: none; border-radius: 5px; transition: .15s; box-shadow: none; outline: none; } input[type=\"text\"]:focus, input[type=\"password\"]:focus{ border-color: #ccc; } button{ display: block; width: 100%; padding: 10px 20px; font-size: 1rem; font-weight: 700; text-transform: uppercase; background: #ff9c29; border: 0; border-radius: 5px; cursor: pointer; transition: .15s; outline: none; } button:hover{ background: #ffba66; } .github{ margin-top: 15px; text-align: center; } .github a{ color: #ff9c29; transition: .15s; } .github a:hover{ color: #ffba66; } </style> <style media=\"all and (max-width: 520px)\"> .wrapper{ margin: 0 0 20px 0; } .container{ padding: 25px 15px; border-radius: 0; } </style> </head> <body> <div class=\"wrapper\"> <div class=\"container\"> <form method=\"post\" action=\"/\"> <h1>Hugh Configuration</h1> <p>Press any of the Hugh's buttons to shut down config AP and resume normal function.</p> <h2>Network settings</h2> <p>Select your network settings here.</p> <div class=\"row\"> <label for=\"ssid\">WiFi SSID</label> <input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"";
  html += json["ssid"].as<const char*>();
  html += "\"> </div> <div class=\"row\"> <label for=\"pass\">WIFI Password</label> <input type=\"password\" id=\"pass\" name=\"pass\" value=\"";
  html += json["pass"].as<const char*>();
  html += "\"> </div> <h2>Static IP settings (optional)</h2> <p>Optional settings for static IP, in some cases this might speed up response time. All 3 need to be set and IP should be reserved in router's DHCP settings.</p> <div class=\"row\"> <label for=\"ip\">IP Address (optional):</label> <input type=\"text\" id=\"ip\" name=\"ip\" value=\"";
  html += json["ip"].as<const char*>();
  html += "\"> </div> <div class=\"row\"> <label for=\"gw\">Gateway IP (optional):</label> <input type=\"text\" id=\"gw\" name=\"gw\" value=\"";
  html += json["gw"].as<const char*>();
  html += "\"> </div> <div class=\"row\"> <label for=\"sn\">Subnet mask (optional):</label> <input type=\"text\" id=\"sn\" name=\"sn\" value=\"";
  html += json["sn"].as<const char*>();
  html += "\"> </div> <h2>diyHue settings</h2> <p>For the remote to work, specify your diyHue bridge IP address.<br>For example: \"192.168.0.100\"</p> <div class=\"row\"> <label for=\"bridge\">Bridge IP</label> <input type=\"text\" id=\"bridge\" name=\"bridge\" value=\"";
  html += json["bridge"].as<const char*>();
  html += "\"> </div> <div class=\"row\"> <button type=\"submit\">Save and reboot</button> </div> </form> </div>";
  html += "<div class=\"github\"><p>diyHue firmware v1.0, check out <a href=\"https://github.com/mcer12/Hugh-ESP8266\" target=\"_blank\"><strong>Hugh Switch</strong> on GitHub</a></p></div>";
  html += "</div> </body> </html>";
  server.send(200, "text/html", html);

  if (server.args()) {
    delay(1000);
    ESP.reset();
    delay(100);
  }
}
