/*
  Lab 7 - Capstone: Telecom Site Monitor

  A remote mobile-network tower site, in miniature. The ESP32 watches:
    - the backup battery's voltage   (dial 1)
    - the generator's fuel level     (dial 2)
    - the equipment shelter's door   (slide switch)
    - the shelter's temperature      (DHT22)
  and it acts on its own, at the site (edge computing, like Lab 5):
    1. Battery low            -> start the generator (relay ON)
    2. Fuel drops while the
       generator is OFF       -> "possible fuel theft" alarm
    3. Door opened            -> alarm
  Every reading and the site status go to the operations centre over MQTT
  (like Lab 3). Watch them in the HiveMQ web client.

  Remember to change YOUR_NAME to something unique before running.

  Libraries used (Wokwi installs them from libraries.txt):
    - DHTesp        (temperature/humidity sensor)
    - PubSubClient  (MQTT)
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

// ---- Wi-Fi + MQTT settings (same as Lab 3) ----
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* YOUR_NAME = "changeme";  // <-- change this to something unique!
String topicBase = "iot-workshop/" + String(YOUR_NAME) + "/site/";

// ---- Pins ----
const int DHT_PIN = 15;
const int BATTERY_PIN = 34;   // dial 1 (ADC1 pin, works with Wi-Fi on)
const int FUEL_PIN = 35;      // dial 2 (ADC1 pin)
const int DOOR_PIN = 4;       // slide switch: left = closed, right = open
const int GENERATOR_PIN = 5;  // relay: switches the generator on
const int BUZZER_PIN = 18;    // local alarm

// ---- The site's rules ----
const float GENERATOR_START_V = 47.0;  // battery below this -> start the generator
const float GENERATOR_STOP_V = 52.0;   // ...and stop it again only above this (hysteresis)
const float THEFT_DROP_PERCENT = 3.0;  // fuel falling this much with the generator OFF is suspicious

DHTesp dhtSensor;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

bool generatorOn = false;
bool theftAlarm = false;
bool doorWasOpen = false;
float lastFuel = -1;

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

void publish(const char* name, String value) {
  mqttClient.publish((topicBase + name).c_str(), value.c_str());
}

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(GENERATOR_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(GENERATOR_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  connectToWiFi();
  connectToBroker();

  Serial.println("Telecom Site Monitor started.");
  Serial.print("Publishing to: ");
  Serial.print(topicBase);
  Serial.println("#");
}

void loop() {
  if (!mqttClient.connected()) {
    connectToBroker();
  }
  mqttClient.loop();

  // 1. Sense: read every sensor and convert to real units
  float battery = 40.0 + analogRead(BATTERY_PIN) * 16.0 / 4095.0;  // a 48 V battery bank: 40-56 V
  float fuel = analogRead(FUEL_PIN) * 100.0 / 4095.0;               // tank level, 0-100 %
  bool doorOpen = digitalRead(DOOR_PIN) == HIGH;                     // switch to the right = open
  float shelter = dhtSensor.getTempAndHumidity().temperature;

  // 2. Decide and act, here at the site
  // Rule 1: battery low -> start the generator; stop it once the battery has recovered
  if (!generatorOn && battery < GENERATOR_START_V) {
    generatorOn = true;
    Serial.println(">> Battery low: generator STARTED");
  } else if (generatorOn && battery > GENERATOR_STOP_V) {
    generatorOn = false;
    Serial.println(">> Battery recovered: generator STOPPED");
  }
  digitalWrite(GENERATOR_PIN, generatorOn ? HIGH : LOW);

  // Rule 2: fuel should only go down while the generator is running
  if (lastFuel >= 0 && !generatorOn && lastFuel - fuel > THEFT_DROP_PERCENT) {
    theftAlarm = true;
    Serial.println(">> ALARM: fuel dropped while the generator was OFF - possible theft");
  }
  if (lastFuel >= 0 && fuel - lastFuel > THEFT_DROP_PERCENT && theftAlarm) {
    theftAlarm = false;  // the tank was refilled: clear the alarm
    Serial.println(">> Tank refilled: theft alarm cleared");
  }
  lastFuel = fuel;

  // Rule 3: the shelter door
  if (doorOpen != doorWasOpen) {
    Serial.println(doorOpen ? ">> ALARM: shelter door OPENED" : ">> Shelter door closed");
    doorWasOpen = doorOpen;
  }

  String status = "normal";
  if (theftAlarm) status = "ALARM: possible fuel theft";
  else if (doorOpen) status = "ALARM: door open";
  else if (generatorOn) status = "generator running";
  digitalWrite(BUZZER_PIN, (theftAlarm || doorOpen) ? HIGH : LOW);

  // 3. Send: report everything to the operations centre
  publish("battery", String(battery, 1));
  publish("fuel", String(fuel, 0));
  publish("shelter-temp", String(shelter, 1));
  publish("door", doorOpen ? "open" : "closed");
  publish("generator", generatorOn ? "on" : "off");
  publish("status", status);

  Serial.print("Battery ");
  Serial.print(battery, 1);
  Serial.print(" V | Fuel ");
  Serial.print(fuel, 0);
  Serial.print(" % | Shelter ");
  Serial.print(shelter, 1);
  Serial.print(" C | Door ");
  Serial.print(doorOpen ? "OPEN" : "closed");
  Serial.print(" | Generator ");
  Serial.print(generatorOn ? "ON" : "off");
  Serial.print(" | Status: ");
  Serial.println(status);

  delay(2000);
}
