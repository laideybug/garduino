#include <ArduinoJson.h>
#include <BH1750.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <RTClib.h>
#include <SoftwareSerial.h>
#include <WiFiEsp.h>
#include <Wire.h>

// Debug Mode
#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINT_LN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINT_LN(x)
#endif

// DHT
#define DHT_PIN 4
DHT dht(DHT_PIN, DHT22);

// BH1750
BH1750 lightMeter;

// Soil Moisture Sensor
#define SOIL_PIN A0

// DS1307
RTC_DS1307 rtc;

// ESP8266
#define RX 12
#define TX 13
SoftwareSerial esp8266(RX,TX);
WiFiEspClient espClient;

// MQTT
#define MQTT_PORT 1883
const char* ssid = ".........";
const char* password = ".........";
const char* mqttUser = ".........";
const char* mqttPass = ".........";
const char* mqttHost = ".........";
const char* mqttPubTopic = "/sensors/status";
const char* mqttClientID = "garduino_01";
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  Serial.println();
  Wire.begin();
  dht.begin();
  lightMeter.begin();
  rtc.begin();
  // Only need to seed the initial RTC time once
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  setupWiFi();
  client.setServer(mqttHost, MQTT_PORT);
  connectMQTTBroker();
}

void loop() {
  if (!client.connected()) {
    connectMQTTBroker();
  }
  client.loop();

  uint32_t t = rtc.now().unixtime();
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  float hic = dht.computeHeatIndex(temp, hum, false);
  float lux = lightMeter.readLightLevel(true);
  float soil = analogRead(SOIL_PIN);

  DEBUG_PRINT(F("[Garduino] Humidity: "));
  DEBUG_PRINT(hum);
  DEBUG_PRINT(F("%  Temperature: "));
  DEBUG_PRINT(temp);
  DEBUG_PRINT(F("°C  Heat index: "));
  DEBUG_PRINT(hic);
  DEBUG_PRINT(F("°C  Light: "));
  DEBUG_PRINT(lux);
  DEBUG_PRINT(F(" lx  Soil moisture level: "));
  DEBUG_PRINT(soil);
  DEBUG_PRINT(F("  Timestamp: "));
  DEBUG_PRINT(t);
  
  StaticJsonDocument<JSON_OBJECT_SIZE(6)> doc;
  doc["hum"] = hum;
  doc["temp"] = temp;
  doc["hic"] = hic;
  doc["lux"] = lux;
  doc["soil"] = soil;
  doc["time"] = t;

  char buffer[256];
  serializeJson(doc, buffer);
  if(client.publish(mqttPubTopic, buffer)) {
    DEBUG_PRINT_LN("  Publish succeeded");
  } else {
    DEBUG_PRINT_LN("  Publish failed");
  }
  
  delay(3000);
}

void setupWiFi() {
  esp8266.begin(9600);
  WiFi.init(&esp8266);

  if (WiFi.status() == WL_NO_SHIELD) {
    while (true);
  }

  DEBUG_PRINT(F("[Garduino] Connecting to "));
  DEBUG_PRINT_LN(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void connectMQTTBroker() {
  while (!client.connected()) {
    if (client.connect(mqttClientID, mqttUser, mqttPass)) {
      DEBUG_PRINT(F("[Garduino] Connected to "));
      DEBUG_PRINT_LN(mqttHost);
    } else {
      DEBUG_PRINT(F("[Garduino] Connection failed, rc="));
      DEBUG_PRINT(client.state());
      DEBUG_PRINT_LN(F(" try again in 5 seconds"));
      delay(5000);
    }
  }
}
