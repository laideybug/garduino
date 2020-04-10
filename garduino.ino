#include <ArduinoJson.h>
#include <BH1750.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <WiFiEsp.h>
#include <Wire.h>

// DHT
#define DHT_PIN 4
DHT dht(DHT_PIN, DHT22);

// BH1750
BH1750 lightMeter;

// ESP8266
#define RX 12
#define TX 13
SoftwareSerial esp8266(RX,TX);
WiFiEspClient espClient;

// MQTT
const char* ssid = ".........";
const char* password = ".........";
const char* mqttUser = ".........";
const char* mqttPass = ".........";
const char* mqttServer = ".........";
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  dht.begin();
  lightMeter.begin();
  setupWiFi();
  client.setServer(mqttServer, 1883);
  connectMQTTBroker();
}

void loop() {
  if (!client.connected()) {
    connectMQTTBroker();
  }
  client.loop();

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

  StaticJsonDocument<128> doc;
  doc["humidity"] = h;
  doc["temperature"]   = t;
  doc["heat_index"] = hic;
  doc["lux"] = lux;

  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  client.publish("outTopic", buffer, n);
  delay(3000);
}

void setupWiFi() {
  delay(10);
  esp8266.begin(9600);
  WiFi.init(&esp8266);

  if (WiFi.status() == WL_NO_SHIELD) {
    while (true);
  }
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void connectMQTTBroker() {
  while (!client.connected()) {
    if (client.connect("Garduino", mqttUser, mqttPass)) {
      Serial.print("Connected to ");
      Serial.println(mqttServer);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
