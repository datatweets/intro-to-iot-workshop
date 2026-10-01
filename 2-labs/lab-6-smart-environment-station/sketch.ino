/*
  Lab 6 - Capstone: Smart Environment Station

  This project puts the whole day together. Nothing here is new - it's
  Lab 2 (DHT22) + Lab 3 (Wi-Fi + MQTT) + Lab 5 (automation) + a small
  screen to show the current status, all in one project. Read the code
  and see if you recognize each part from earlier labs.

  Remember to change YOUR_NAME to something unique before running,
  just like in Lab 3.

  Libraries used (Wokwi will offer to install these automatically):
    - DHTesp            (temperature/humidity sensor)
    - PubSubClient       (MQTT)
    - Adafruit_SSD1306   (the small OLED screen)
    - Adafruit_GFX       (drawing text/shapes on the screen)
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h"

// ---- Wi-Fi + MQTT settings (same as Lab 3) ----
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* YOUR_NAME = "changeme";  // <-- change this to something unique!
String topicTemperature = "iot-workshop/" + String(YOUR_NAME) + "/temperature";
String topicHumidity    = "iot-workshop/" + String(YOUR_NAME) + "/humidity";
String topicStatus      = "iot-workshop/" + String(YOUR_NAME) + "/status";

// ---- Pins ----
const int DHT_PIN = 15;
const int RELAY_PIN = 5;
const int BUZZER_PIN = 18;

// ---- Automation rule (same idea as Lab 5) ----
const float TEMPERATURE_THRESHOLD_C = 28.0;

// ---- Screen setup ----
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DHTesp dhtSensor;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
bool alarmIsOn = false;

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" connected!");
}

void connectToBroker() {
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  while (!mqttClient.connected()) {
    String clientId = "esp32-" + String(YOUR_NAME) + "-" + String(random(0, 10000));
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("MQTT connected!");
    } else {
      delay(2000);
    }
  }
}

void showOnScreen(float temperature, float humidity, bool alarm) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Smart Environment Station");
  display.println("--------------------------");

  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print(temperature, 1);
  display.println(" C");

  display.setTextSize(1);
  display.setCursor(0, 45);
  display.print("Humidity: ");
  display.print(humidity, 0);
  display.println(" %");

  display.setCursor(0, 55);
  display.println(alarm ? "STATUS: ALARM!" : "STATUS: normal");

  display.display();
}

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  connectToWiFi();
  connectToBroker();

  Serial.println("Capstone project started.");
}

void loop() {
  if (!mqttClient.connected()) {
    connectToBroker();
  }
  mqttClient.loop();

  // 1. Read the sensor (Lab 2)
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  // 2. Decide locally whether to alarm (Lab 5 - edge computing)
  bool alarm = data.temperature > TEMPERATURE_THRESHOLD_C;
  digitalWrite(RELAY_PIN, alarm ? HIGH : LOW);
  digitalWrite(BUZZER_PIN, alarm ? HIGH : LOW);

  if (alarm != alarmIsOn) {
    Serial.println(alarm ? "ALARM triggered!" : "Back to normal.");
    alarmIsOn = alarm;
  }

  // 3. Show the current status locally (new in this lab)
  showOnScreen(data.temperature, data.humidity, alarm);

  // 4. Publish to the cloud too (Lab 3), so it's visible remotely
  mqttClient.publish(topicTemperature.c_str(), String(data.temperature, 1).c_str());
  mqttClient.publish(topicHumidity.c_str(), String(data.humidity, 1).c_str());
  mqttClient.publish(topicStatus.c_str(), alarm ? "ALARM" : "normal");

  Serial.print("Temp: ");
  Serial.print(data.temperature, 1);
  Serial.print(" C | Humidity: ");
  Serial.print(data.humidity, 1);
  Serial.print(" % | Status: ");
  Serial.println(alarm ? "ALARM" : "normal");

  delay(3000);
}
