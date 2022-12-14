// Configuration
#include "config.h"
// #define WIFI_SSID
// #define WIFI_PASSWORD
// #define DHT_PIN
// #define DHT_TYPE
// #define DHT_INTERVAL_S
// #define INFLUXDB_URL
// #define INFLUXDB_TOKEN
// #define INFLUXDB_ORG
// #define INFLUXDB_BUCKET
// #define TZ_INFO

// WiFi
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;

// InfluxDB
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
InfluxDBClient influxClient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET,
                            INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Temp sensor
#include "DHT.h"
DHT dht(DHT_PIN, DHT_TYPE);
Point tempSensor("temp_sensor");

void setup() {
  Serial.begin(115200);
  dht.begin();

  Serial.println("Hello!");

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  if (influxClient.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(influxClient.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(influxClient.getLastErrorMessage());
  }
}

void loop() {
  delay(DHT_INTERVAL_S * 1000);

  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
    return;
  }

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  tempSensor.clearFields();
  tempSensor.addField("temperature", t);
  tempSensor.addField("humidity", h);

  if (!influxClient.writePoint(tempSensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(influxClient.getLastErrorMessage());
    return;
  }
}
