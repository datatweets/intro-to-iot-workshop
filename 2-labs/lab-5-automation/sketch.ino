/*
  Lab 5 - Automation: threshold-triggered relay + buzzer

  This is "edge computing" made visible: the ESP32 decides on its own,
  right here, whether to switch on a relay and sound a buzzer - based on
  a temperature reading - with no cloud, no internet, no waiting for a
  server to answer.

  The relay stands in for any real-world appliance: a fan, a heater, an
  irrigation valve, a warning light. The LED wired through the relay's
  NO ("normally open") and COM pins represents that appliance switching on.
*/

#include "DHTesp.h"

const int DHT_PIN = 15;
const int RELAY_PIN = 5;
const int BUZZER_PIN = 18;

// The rule: if temperature goes above this, react.
const float TEMPERATURE_THRESHOLD_C = 28.0;

DHTesp dhtSensor;
bool alarmIsOn = false;

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("Lab 5 started.");
  Serial.print("Rule: if temperature > ");
  Serial.print(TEMPERATURE_THRESHOLD_C);
  Serial.println(" C, switch on the relay and buzzer.");
  Serial.println("Click the DHT22 sensor and raise the temperature slider to test it.");
}

void loop() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  Serial.print("Temperature: ");
  Serial.print(data.temperature, 1);
  Serial.print(" C   |   ");

  // ---- The automation rule ----
  if (data.temperature > TEMPERATURE_THRESHOLD_C) {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    if (!alarmIsOn) {
      Serial.println("THRESHOLD CROSSED -> relay ON, buzzer ON");
      alarmIsOn = true;
    } else {
      Serial.println("(still above threshold)");
    }
  } else {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    if (alarmIsOn) {
      Serial.println("back to normal -> relay OFF, buzzer OFF");
      alarmIsOn = false;
    } else {
      Serial.println("normal");
    }
  }

  delay(2000);
}
