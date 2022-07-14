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
String preferedLine = "31";          // Display can only show one line at a time. This is the prefered line. Could be extended with another button to change prefered line.

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
      DynamicJsonDocument doc(4096 * 2);
      DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(15));
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      
      for (JsonObject monitor : doc["data"]["monitors"].as<JsonArray>())
      {
        const char *stationTitle = monitor["locationStop"]["properties"]["title"]; // title: Station Name
        JsonObject line = monitor["lines"][0];
        const char *lineName = line["name"];   // "31"
        const char *towards = line["towards"]; // "Schottenring U"
        bool trafficjam = line["trafficjam"];  // false
        int countdown0 = line["departures"]["departure"][0]["departureTime"]["countdown"];
        int countdown1 = line["departures"]["departure"][1]["departureTime"]["countdown"];

        if (String(lineName) == preferedLine)
        {
          Serial.println(String(lineName) + "\t" + String(towards) + "\t" + String(countdown0) + "|" + String(countdown1));
          display.clearDisplay();
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(0, 0);
          display.setTextSize(2);
          display.println(String(lineName));
          display.setTextSize(1);
          display.println(String(towards));
          display.setTextSize(4);
          display.println(String(countdown0) + "|" + String(countdown1));
          display.display();
        }
      }
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

  display.setRotation(2); // uncomment to reset orientation
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
