/*
  Lab 4 - Cloud dashboard with ThingSpeak

  This lab sends DHT22 readings to ThingSpeak, a free cloud platform that
  turns your numbers into a live graph automatically - no dashboard coding
  needed. ThingSpeak speaks plain HTTP, which is why this lab uses
  HTTPClient instead of MQTT.

  Before running this: create a free ThingSpeak channel with two fields
  (Field 1 = Temperature, Field 2 = Humidity), then copy your channel's
  "Write API Key" into API_KEY below. See this lab's README for the full
  step-by-step.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include "DHTesp.h"

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// Paste your ThingSpeak channel's Write API Key here:
const char* API_KEY = "YOUR_WRITE_API_KEY";

const int DHT_PIN = 15;
DHTesp dhtSensor;

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" connected!");
}

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  connectToWiFi();
}

void loop() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  // ThingSpeak's free plan needs at least 15 seconds between updates to
  // the same channel - we use 20 to stay comfortably clear of that limit.
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "http://api.thingspeak.com/update?api_key=" + String(API_KEY) +
                 "&field1=" + String(data.temperature, 1) +
                 "&field2=" + String(data.humidity, 1);

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      Serial.println("Sent to ThingSpeak: temp=" + String(data.temperature, 1) +
                      "  humidity=" + String(data.humidity, 1));
    } else {
      Serial.print("ThingSpeak request failed, HTTP code: ");
      Serial.println(httpResponseCode);
      Serial.println("Check that API_KEY is correct.");
    }

    http.end();
  } else {
    Serial.println("Wi-Fi not connected.");
  }

  delay(20000);
}
