#include <BH1750.h>
#include "DHT.h"
#include <Wire.h>

// DHT
#define DHT_PIN 4

DHT dht(DHT_PIN, DHT22);

// BH1750
BH1750 lightMeter;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  dht.begin();
  lightMeter.begin();
}

void loop() {
  delay(2000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float hic = dht.computeHeatIndex(t, h, false);
  float lux = lightMeter.readLightLevel(true);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C  Light: "));
  Serial.print(lux);
  Serial.println(" lx");
}
