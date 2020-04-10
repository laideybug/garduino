#include <ArduinoJson.h>
#include <BH1750.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <RTClib.h>
#include <SoftwareSerial.h>
#include <WiFiEsp.h>
#include <Wire.h>

// DHT
#define DHT_PIN 4
DHT dht(DHT_PIN, DHT22);

// BH1750
BH1750 lightMeter;

// DS1307
RTC_DS1307 rtc;

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
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  setupWiFi();
  client.setServer(mqttServer, 1883);
  connectMQTTBroker();
}

void loop() {
  if (!client.connected()) {
    connectMQTTBroker();
  }
  client.loop();

  uint32_t tim = rtc.now().unixtime();
  float hum = dht.readHumidity();
  float tmp = dht.readTemperature();
  float hic = dht.computeHeatIndex(tmp, hum, false);
  float lux = lightMeter.readLightLevel(true);

  Serial.print(F("Humidity: "));
  Serial.print(hum);
  Serial.print(F("%  Temperature: "));
  Serial.print(tmp);
  Serial.print(F("°C  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C  Light: "));
  Serial.print(lux);
  Serial.print(" lx");
  Serial.print("  Timestamp: ");
  Serial.println(tim);

  StaticJsonDocument<140> doc;
  doc["humidity"] = hum;
  doc["temperature"]   = tmp;
  doc["heat_index"] = hic;
  doc["lux"] = lux;
  doc["timestamp"] = tim;

  char buffer[140];
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
