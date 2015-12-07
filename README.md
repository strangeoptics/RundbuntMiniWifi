# RundbuntMiniWifi

## Arduino
The project consists of two parts. The actual arduino sketch RundBuntWifi.ino and the files for the web front end in the /data folder.

For the sketch you need to install the esp8266 board extentions for arduino https://github.com/esp8266/Arduino.

For the web front end you need to install the tool ESP8266FS from here https://github.com/esp8266/arduino-esp8266fs-plugin/.
The tool enables you to upload the files in the /data folder into the flash chip of the board.

## Processing
This is a little simulation sketch for the LED-Matrix. It allows to develop new pattern algorithms to control the leds whitout the hassel to compile and upload it to the esp8266 which takes ~30-40 seconds.
