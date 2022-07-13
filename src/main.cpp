/*
Author: Dominik Höbert, MSc
Datum: 2022-07-10

Funktion:
  - Fragt wienerlinien API nach den Echtzeitabfahren einer Station ab
  - Zeigt die nächsten zwei Abfahren auf einem Mini OLED Display an
  - Button Druck wechselt stationen
  - Deep Sleep nach bestimmter Zeit
  - Button weckt aus Sleep auf

Pins:
  Button:
    - Button --> 4 buttonPin
    - Button --> GND

  0,97" Mini OLED SSD1306:
      OLED  |  ESP32
    -  GND      GND
    -  VCC      3V3
    -  SCL      22 (SCL)
    -  SDA      21 (SDA)

Credentails:
  Create a credentails.h file with the folloing content:
  #define SSID "your-wifi-ssid"
  #define WIFIpassword "your-wifi-password"

Librarys:
  - ArduinoJson https://arduinojson.org/
  - Adafruit_SSD1306

Other:
  - Pinout: https://www.studiopieters.nl/esp32-pinout/
  - Deep Sleep: https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/
  - HTTP Get: https://randomnerdtutorials.com/esp32-http-get-post-arduino/
  - OLED: https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/
  - Wiener Linien API: https://www.data.gv.at/katalog/dataset/wiener-linien-echtzeitdaten-via-datendrehscheibe-wien
  - Wiener Linien Station Numbers: https://till.mabe.at/rbl/

*/

// TODO:
// - filter for lines (e.g. "31"), or by button
// - RGB LED traffic light

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <credentials.h>

const char *ssid = SSID;
const char *password = WIFIpassword;

String serverName = "http://www.wienerlinien.at/ogd_realtime/monitor?rbl=";
int stations[] = {2171, 2190}; // https://till.mabe.at/rbl/

// IF BUTTON PIN IS CHANGED esp_sleep_enable_ext0_wakeup(GPIO_NUM_4,0); IN setup() HAS TO BE CHANGED TOO!!!
const int buttonPin = 4; // the number of the pushbutton pin

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Delay until next GET Request
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;          // time until next request (update)
unsigned long timetosleep = 5 * 60 * 1000; // time until deep sleep

int choice = 0;

// Button debounce variables
int buttonState;                    // the current reading from the input pin
int lastButtonState = HIGH;         // the previous reading from the input pin
unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 50;   // the debounce time; increase if the output flickers

void request_station()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClient client;
    HTTPClient http;
    String serverPath = serverName + String(stations[choice]);
    http.begin(client, serverPath.c_str());
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0)
    {
      String payload = http.getString();
      StaticJsonDocument<0> filter;
      filter.set(true);
      DynamicJsonDocument doc(4096);
      DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(15));
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      JsonObject data_monitors_0 = doc["data"]["monitors"][0];
      JsonObject data_monitors_0_locationStop = data_monitors_0["locationStop"];
      const char *data_monitors_0_locationStop_geometry_type = data_monitors_0_locationStop["geometry"]["type"];
      JsonObject data_monitors_0_locationStop_properties = data_monitors_0_locationStop["properties"];
      const char *data_monitors_0_locationStop_properties_title = data_monitors_0_locationStop_properties["title"];
      JsonObject data_monitors_0_lines_0 = data_monitors_0["lines"][0];
      const char *data_monitors_0_lines_0_name = data_monitors_0_lines_0["name"];                    // "31"
      const char *data_monitors_0_lines_0_towards = data_monitors_0_lines_0["towards"];              // "Schottenring U"
      const char *data_monitors_0_lines_0_direction = data_monitors_0_lines_0["direction"];          // "R"
      const char *data_monitors_0_lines_0_platform = data_monitors_0_lines_0["platform"];            // "2"
      const char *data_monitors_0_lines_0_richtungsId = data_monitors_0_lines_0["richtungsId"];      // "2"
      bool data_monitors_0_lines_0_barrierFree = data_monitors_0_lines_0["barrierFree"];             // true
      bool data_monitors_0_lines_0_realtimeSupported = data_monitors_0_lines_0["realtimeSupported"]; // true
      bool data_monitors_0_lines_0_trafficjam = data_monitors_0_lines_0["trafficjam"];               // false
      int countdown0 = data_monitors_0_lines_0["departures"]["departure"][0]["departureTime"]["countdown"];
      int countdown1 = data_monitors_0_lines_0["departures"]["departure"][1]["departureTime"]["countdown"];
      int countdown2 = data_monitors_0_lines_0["departures"]["departure"][2]["departureTime"]["countdown"];
      Serial.println(String(data_monitors_0_lines_0_name) + "\t" + String(data_monitors_0_lines_0_towards) + "\t" + String(countdown0) + "|" + String(countdown1));
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      // display.setFont(&FreeSans9pt7b);
      display.setCursor(0, 0);
      display.setTextSize(2);
      display.println(String(data_monitors_0_lines_0_name));
      display.setTextSize(1);
      display.println(String(data_monitors_0_lines_0_towards));
      display.setTextSize(4);
      display.println(String(countdown0) + "|" + String(countdown1));
      display.display();
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }
  lastTime = millis();
}

void testdrawstyles(void)
{
  display.clearDisplay();

  display.setTextSize(1);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);             // Start at top-left corner
  display.println(F("Hello, world!"));

  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  display.println(3.141592);

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.print(F("0x"));
  display.println(0xDEADBEEF, HEX);

  display.display();
}

void setup()
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.clearDisplay();

  // testdrawstyles();
  display.clearDisplay();
  display.setTextSize(1);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);             // Start at top-left corner
  display.println("Connecting to WiFi " + String(ssid));
  display.display();

  pinMode(buttonPin, INPUT_PULLUP);

  // CHANGE GPIO_NUM_X WITH BUTTONPIN
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0); // 1 = High, 0 = Low
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  display.clearDisplay();
  display.println("CONNECTED!");
  display.println(String(WiFi.localIP()));
  request_station();
}

void loop()
{
  // activate deep sleep
  if (millis() > timetosleep)
  {
    Serial.println("Going to sleep now");
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(3);
    display.println("SLEEPMODE");
    display.display();
    delay(1000);
    display.clearDisplay();
    display.display();
    delay(1000);
    esp_deep_sleep_start();
  }

  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState)
  {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    if (reading != buttonState)
    {
      buttonState = reading;
      if (buttonState == LOW)
      {
        choice++;
        if (choice >= sizeof(stations) / sizeof(stations[0]))
        {
          choice = 0;
        }
        request_station();
      }
    }
  }
  lastButtonState = reading;

  if ((millis() - lastTime) > timerDelay)
  {
    request_station();
  }
}
