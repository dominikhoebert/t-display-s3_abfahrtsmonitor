# Wiener Linien Abfahrtsmonitor for ESP32

*Author:* Dominik Höbert, MSc  
*Date:* 2022-07-10

## Funktion:

  - Fragt wienerlinien API nach den Echtzeitabfahren einer Station ab
  - Zeigt die nächsten zwei Abfahren auf einem Mini OLED Display an
  - Button Druck wechselt stationen
  - Deep Sleep nach bestimmter Zeit
  - Button weckt aus Sleep auf


## Pins

|  Device |  ESP32 |
|---|---|
|Button   |4 (buttonPin)   |
|Button   |GND   |
|   |   |
|0,97" Mini OLED SSD1306|
|GND   |GND   |
|VCC   |3V3   |
|SCL   |22 (SCL)   |
|SDA   |21 (SDA)  |

## Credentails:

Create a credentails.h file with the folloing content:

```cpp
#define SSID "your-wifi-ssid"
#define WIFIpassword "your-wifi-password"
```

## Librarys:

  - ArduinoJson https://arduinojson.org/
  - Adafruit_SSD1306

## Other:

  - Pinout: https://www.studiopieters.nl/esp32-pinout/
  - Deep Sleep: https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/
  - HTTP Get: https://randomnerdtutorials.com/esp32-http-get-post-arduino/
  - OLED: https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/
  - Wiener Linien API: https://www.data.gv.at/katalog/dataset/wiener-linien-echtzeitdaten-via-datendrehscheibe-wien
  - Wiener Linien Station Numbers: https://till.mabe.at/rbl/

## TODO:

 - filter for lines (e.g. "31"), or by button
 - RGB LED traffic light

 

