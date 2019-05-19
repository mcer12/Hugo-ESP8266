# Hugh-ESP8266
## First run
1) Push any of the buttons. Led indicator will start blinking fast and new wifi access point will appear for 5 minutes. 
2) Connect to the network with phone or PC and open URL 10.10.10.0
3) There you will set your wifi credentials and url address for each button.
4) (Optional) To further improve performance and battery life, you can add your router's gateway IP address and also pick a static IP for your remote. You will also have to set the static IP on your router.

![alt text](https://raw.githubusercontent.com/mcer12/Hugh-ESP8266/develop/Images/ap_screen.png)

## Changing settings
1) Push and hold button 1 and 4 until the led starts blinking and release. Configuration access point will show up for 5 minutes
2) To exit configuration portal, either save changes or press any of the buttons.

## Flashing custom FW

### A) USB
1) Connect the device to your PC via microUSB connector
2) Choose appropriate COM port in arduino IDE and select correct settings as shown in image below.
3) Upload the sketch. The integrated CH340 chip should handle up to 921600 baud but if you're having issues, lower the baud rate to 115200

![alt text](https://raw.githubusercontent.com/mcer12/Hugh-ESP8266/develop/Images/ide_settings.png)

### B) Arduino OTA
1) Push the 2nd and 3rd button at the same time and hold them for about 3 seconds, then release. Led indicator should start blinking.
2) Open arduino IDE adn in available ports list there should be your remote.
3) Upload your sketch as usual

### C) SERIAL BREAKOUT
Once you open up the enclosure, you will find the usual breakout pins for ESP8266: VCC, GND, RX, TX, GPIO0. You can solder a header to them but it won't fit in the enclosure anymore so I advise to solder the wires directly if you decide to go with this option.
Do NOT connect VCC to external power source!
