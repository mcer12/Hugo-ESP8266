void startOTA() {
  startBlinking(OTA_BLINK_SPEED);

  delay(3000);
  
  while (millis() - otaTimer < OTA_TIMEOUT) {
    if (digitalRead(button1_pin) == HIGH || digitalRead(button2_pin) == HIGH || digitalRead(button3_pin) == HIGH || digitalRead(button4_pin) == HIGH) {
      stopBlinking();
      goToSleep();
      return;
    }
    ArduinoOTA.handle();
    delay(20);
  }
  stopBlinking();
  goToSleep();
}

void toggleOTAMode() {
  if (digitalRead(button1_pin) == HIGH && digitalRead(button3_pin) == HIGH) {
    int i = 0;
    while (digitalRead(button1_pin) == HIGH && digitalRead(button3_pin) == HIGH && i < 200) {
      delay(10);

      if (i > 100) {
        deviceMode = OTA_MODE;
        otaTimer = millis(); // start counter
        return;
      }
      i++;
    }
  }
}
