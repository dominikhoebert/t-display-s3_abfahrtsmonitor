# Wiener Linien Abfahrtsmonitor for LILYGO T-DISPLAY-S3

*Author:* Dominik Höbert, MSc  
*Date:* 2022-08-19

## Funktion:

  - Fragt wienerlinien API nach den Echtzeitabfahren einer Station ab
  - Zeigt die nächsten zwei Abfahren auf einem Mini OLED Display an
  - Deep Sleep nach bestimmter Zeit
  - Button weckt aus Sleep auf

## Credentails:

Create a credentails.h file with the folloing content:

```cpp
#define SSID "your-wifi-ssid"
#define WIFIpassword "your-wifi-password"
```

## Librarys:

  - ArduinoJson https://arduinojson.org/
  - OneButton
  - TFT eSPI

## Other:

  - LILYGO T-DISPLAY-S3: https://github.com/Xinyuan-LilyGO/T-Display-S3
  - Deep Sleep: https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/
  - HTTP Get: https://randomnerdtutorials.com/esp32-http-get-post-arduino/
  - OLED: https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/
  - Wiener Linien API: https://www.data.gv.at/katalog/dataset/wiener-linien-echtzeitdaten-via-datendrehscheibe-wien
  - Wiener Linien Station Numbers: https://till.mabe.at/rbl/

## TODO

- ~~prettier connecting screen~~
- ~~show current time~~
