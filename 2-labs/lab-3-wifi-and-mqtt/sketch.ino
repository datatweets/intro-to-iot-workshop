/*
  Lab 3 - Getting online: Wi-Fi + MQTT

  This lab connects the simulated ESP32 to the internet, then publishes
  live DHT22 readings to a public MQTT broker, so you can watch your own
  data arrive in real time on a website.

  IMPORTANT: change YOUR_NAME below to something unique (e.g. your first
  name). We're all sharing one public test broker - if two people use the
  same topic name, your messages will mix together on screen.

  Library used: PubSubClient (by Nick O'Leary). Wokwi will offer to install
  it automatically the first time you press Play.
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"

// ---- 1. Wi-Fi settings ----
// Wokwi's simulated Wi-Fi network. No password needed. This gives the
// simulated ESP32 real internet access.
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// ---- 2. MQTT settings ----
const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

const char* YOUR_NAME = "changeme";  // <-- change this to something unique!
String topicTemperature = "iot-workshop/" + String(YOUR_NAME) + "/temperature";
String topicHumidity    = "iot-workshop/" + String(YOUR_NAME) + "/humidity";

// ---- 3. Sensor setup ----
const int DHT_PIN = 15;
DHTesp dhtSensor;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" connected!");
  Serial.print("Device IP address: ");
  Serial.println(WiFi.localIP());
}

void connectToBroker() {
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker...");
    // A random-ish client ID so multiple learners don't clash.
    String clientId = "esp32-" + String(YOUR_NAME) + "-" + String(random(0, 10000));
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println(" connected!");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" - retrying in 2 seconds");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  connectToWiFi();
  connectToBroker();

  Serial.println();
  Serial.println("Publishing to these topics - watch them at:");
  Serial.println("http://www.hivemq.com/demos/websocket-client/");
  Serial.println(topicTemperature);
  Serial.println(topicHumidity);
}

void loop() {
  if (!mqttClient.connected()) {
    connectToBroker();
  }
  mqttClient.loop();

  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  // MQTT messages are just text. Convert numbers to strings before sending.
  String tempPayload = String(data.temperature, 1);
  String humPayload = String(data.humidity, 1);

  mqttClient.publish(topicTemperature.c_str(), tempPayload.c_str());
  mqttClient.publish(topicHumidity.c_str(), humPayload.c_str());

  Serial.println("Published: " + tempPayload + " C, " + humPayload + " %");

  delay(3000);
}
