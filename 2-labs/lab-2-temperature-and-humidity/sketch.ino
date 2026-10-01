/*
  Lab 2 - Reading a real sensor: DHT22 (temperature + humidity)

  The DHT22 is a very common, cheap sensor used in real weather stations,
  greenhouses, and smart thermostats. It reports two values at once:
  temperature (in Celsius) and relative humidity (in %).

  Library used: DHTesp ("DHT sensor library for ESPx" by beegee_tokyo).
  Wokwi will offer to install it automatically the first time you press
  Play - click "Install" / "OK" if you see a popup. This may take a few
  extra seconds the first time.
*/

#include "DHTesp.h"

const int DHT_PIN = 15;
DHTesp dhtSensor;

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  Serial.println("Lab 2 started.");
  Serial.println("Click the DHT22 sensor in the diagram while the");
  Serial.println("simulation is running to open sliders and change the");
  Serial.println("temperature and humidity live.");
}

void loop() {
  // The DHT22 can only be read about once every 2 seconds - reading it
  // faster than that just returns stale or invalid data.
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  if (dhtSensor.getStatus() != 0) {
    Serial.println("Sensor read failed - check wiring.");
  } else {
    Serial.print("Temperature: ");
    Serial.print(data.temperature, 1);
    Serial.print(" C   |   Humidity: ");
    Serial.print(data.humidity, 1);
    Serial.println(" %");
  }

  delay(2000);
}
