#include <WiFi.h>
#include <string.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <OneButton.h>
#include <TFT_eSPI.h>
#include <pin_config.h>
#include <credentials.h>

const char *ssid = SSID;
const char *password = WIFIpassword;

String serverName = "http://www.wienerlinien.at/ogd_realtime/monitor?stopId=2171&stopId=2190";
String preferedLine = "31"; // Display can only show one line at a time. This is the prefered line. Could be extended with another button to change prefered line.

// Delay until next GET Request
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;          // time until next request (update)
unsigned long timetosleep = 5 * 60 * 1000; // time until deep sleep

TFT_eSPI tft = TFT_eSPI();
OneButton button0(PIN_BUTTON_1, true, false);
OneButton button1(PIN_BUTTON_2, true, false);
OneButton button2(PIN_EXT_BUTTON, true, true);

void request_station()
{
  if (WiFi.status() == WL_CONNECTED)
  {

    WiFiClient client;
    HTTPClient http;
    String serverPath = serverName;
    http.begin(client, serverPath.c_str());
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0)
    {
      String payload = http.getString();
      StaticJsonDocument<0> filter;
      filter.set(true);
      DynamicJsonDocument doc(4096 * 3);
      DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(15));
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      int direction = 0;
      tft.fillScreen(TFT_BLACK);
      String time = doc["message"]["serverTime"];
      time = time.substring(11, 16);
      Serial.println(time);
      tft.setTextColor(TFT_WHITE);
      tft.setFreeFont(&FreeSans12pt7b);
      tft.setTextSize(1);
      tft.drawString(time, 250, 0);

      for (JsonObject monitor : doc["data"]["monitors"].as<JsonArray>())
      {
        const char *stationTitle = monitor["locationStop"]["properties"]["title"]; // title: Station Name
        JsonObject line = monitor["lines"][0];
        const char *lineName = line["name"];   // "31"
        const char *towards = line["towards"]; // "Schottenring U"
        bool trafficjam = line["trafficjam"];  // false
        int countdown0 = line["departures"]["departure"][0]["departureTime"]["countdown"];
        int countdown1 = line["departures"]["departure"][1]["departureTime"]["countdown"];
        int countdown2 = line["departures"]["departure"][2]["departureTime"]["countdown"];

        if (String(lineName) == preferedLine)
        {

          Serial.println(String(lineName) + "\t" + String(towards) + "\t" + String(countdown0) + "|" + String(countdown1));
          String trafficjam_indicator = " ";
          if (trafficjam)
          {
            trafficjam_indicator = " !!";
          }
          tft.setFreeFont(&FreeSans12pt7b);
          tft.setTextSize(1);
          tft.setTextColor(TFT_RED);
          tft.drawString(String(lineName) + trafficjam_indicator, 5, direction * 90);
          tft.setTextColor(TFT_WHITE);
          tft.drawString(String(towards), 35, direction * 90);
          tft.setFreeFont(&FreeSans18pt7b);
          tft.setTextSize(2);
          tft.drawString(String(countdown0) + " / " + String(countdown1) + " / " + String(countdown2), 5, 20 + direction * 90);
          tft.setFreeFont(&FreeSans12pt7b);
          direction++;
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

static void handleClick()
{
  request_station();
}

void setup()
{
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, LOW);
  tft.begin();
  tft.setRotation(3);
  tft.setTextSize(1);
  tft.setFreeFont(&FreeSans18pt7b);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  pinMode(PIN_EXT_BUTTON, INPUT_PULLUP);

  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTON_1, 0);   // 1 = High, 0 = Low
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BUTTON_2, 0);   // 1 = High, 0 = Low
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_EXT_BUTTON, 0); // 1 = High, 0 = Low

  // Single Click event attachment
  button0.attachClick(handleClick);
  button1.attachClick(handleClick);
  button2.attachClick(handleClick);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  // tft.drawString("Connecting to " + String(ssid), 0, 0);
  int i = 0;
  String spinner[5] = {"Ooooo", "oOooo", "ooOoo", "oooOo", "ooooO"};
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    tft.setTextSize(2);
    for (int i = 0; i < 5; i++)
    {
      tft.fillScreen(TFT_BLACK);
      tft.drawString(spinner[i], 50, 50);
      delay(50);
    }
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  // tft.drawString("Connected", 0, 40, 2);
  // tft.drawString(WiFi.localIP().toString(), 0, 60, 2);
  request_station();
}

void loop()
{
  button0.tick();
  button1.tick();

  // activate deep sleep
  if (millis() > timetosleep)
  {
    Serial.println("Going to sleep now");
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("zzZZZzz", 50, 60);
    delay(2000);
    tft.fillScreen(TFT_BLACK);
    pinMode(PIN_POWER_ON, OUTPUT);
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_POWER_ON, LOW);
    digitalWrite(PIN_LCD_BL, LOW);
    delay(1000);
    esp_deep_sleep_start();
  }

  if ((millis() - lastTime) > timerDelay)
  {
    request_station();
  }
}
