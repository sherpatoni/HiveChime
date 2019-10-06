# HiveChime
Repo for hivechime arduino IDE


This is the Arduino IDE script for the HiveChime with active sensors with a NodeMCU board. 
Eventually will add other options.

You will need a micro-USB to connect the NodeMCU to computer as well as an installed Arduino IDE.
https://www.arduino.cc/en/main/software

You will have to add the NodeMCU board to the basic Arduino install.
1. Open you IDE and click on "File -> Preferences".
2. In  "Aditional Boards Manager URLs" add this line and click on "OK":
"http://arduino.esp8266.com/stable/package_esp8266com_index.json"
3.Go to "Tools -> Board -> Boards Manager", type "ESP8266" and install it.

When flashing a board, it is a good idea to check the port and the board choice in "Tools -> Board" 
For NodeMCU select "Generic ESP8266 Module".

You also need some libraries for the code as is now. Libraries can be added in different ways:
1. either directly through the Arduino IDE interface: Sketch -> Include Library -> Manage Libraries
2. find the library in zip on githug or other, and install it with the Sketch -> Include Library -> Add .ZIP library

At the moment, I think you only need the NTPclient library
https://travis-ci.org/arduino-libraries/NTPClient
The code might work with other NTP libraries for arduino.
I may be wrong and you may also need libraries for the <WiFiUdp.h> and <ESP8266WiFi.h> files. I will try to get a new fresh install on my machine as right now it's a bit of a mess. 

Finally, the only change to make to the current script is for wifi credentials on lines 10 and 11.




