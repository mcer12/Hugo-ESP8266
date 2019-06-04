# Hugh-ESP8266

Welcome to official git rep for Hugh, universal wifi remote.  
![alt text](https://raw.githubusercontent.com/mcer12/Hugh-ESP8266/develop/Images/hugh.jpg)

## Useful information
**IMPORTANT FOR FIRMWARE 1.0 USERS:** If you're unable to connect to config AP, update to latest firmware version.  
3D printable case available here:  
https://www.thingiverse.com/thing:3641618  
If you use diyHue emulator, there is custom sketch in Firmwares directory just for that!  
diyHue github: https://github.com/diyhue/diyHue  

## First run
1) Push any of the buttons. Led indicator will start blinking and new wifi access point will appear for 5 minutes. 
2) Connect to the network with phone or PC and open URL 10.10.10.0
3) There you will set your wifi credentials and url address for each button.
4) (Optional) To further improve performance and battery life, you can add your router's gateway IP address and also pick a static IP for your remote. You will also have to set the static IP on your router.

![alt text](https://raw.githubusercontent.com/mcer12/Hugh-ESP8266/develop/Images/ap_screen.png)

## Changing settings
1) Push and hold button 1 and 4 until the led starts blinking and release. Configuration access point will show up for 5 minutes
2) To exit configuration portal, either save changes or press any of the buttons.

## Flashing custom FW
You can flash your own Arduino sketch. 

### A) USB
1) Connect the device to your PC via microUSB connector
2) Choose appropriate COM port in arduino IDE and select correct settings as shown in image below.
3) Upload the sketch. The integrated CH340 chip should handle up to 921600 baud but if you're having issues, lower the baud rate to 115200

![alt text](https://raw.githubusercontent.com/mcer12/Hugh-ESP8266/develop/Images/ide_settings.png)

### B) Arduino OTA
1) Push the 2nd and 3rd button at the same time and hold them until the led starts blinking fast and release. Led indicator should start blinking.
2) Open arduino IDE adn in available ports list there should be your remote.
3) Upload your sketch as usual

### C) SERIAL BREAKOUT
Once you open up the enclosure, you will find the usual breakout pins for ESP8266: VCC, GND, RX, TX, GPIO0. You can solder a header to them but it won't fit in the enclosure anymore so I advise to solder the wires directly if you decide to go with this option.
Do NOT connect VCC to external power source!

## Useful information for making custom sketch
1) you need to set GPIO16 low at the beginning of setup() if you want to use buttons in your sketch. Otherwise anytime you push any button, Hugh will restart.
pinMode(16, OUTPUT);  
digitalWrite(16, LOW);  

## CREDITS
Marius Motea and his diyHue project (https://github.com/diyhue/diyHue). His project was the reason to design the remote in the first place and firmware sketch was initially based on it.