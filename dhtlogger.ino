#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

ESP8266WiFiMulti wifiMulti;

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("Morgans WiFi", "mycaptain");
  wifiMulti.addAP("M2", "mycaptain");

  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi not yet connected.");
    //Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    //ESP.restart();
  }
}


void setupOTA() {
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void loopOTA() {
  ArduinoOTA.handle();
}


#include "DHT.h"
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
void setupDht() {
  dht.begin();
}

WiFiClient client;

const char* logstashHost = "192.168.0.44";
const int logstashPort = 12345;
void loopLogstash() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!client.connected() && !client.connect(logstashHost, logstashPort)) {
    Serial.println("logstash connect failed");
    return;
  }

  if (isnan(h) || isnan(t)) {
    Serial.println("dht read failed");
    client.println("{\"esp.chipid\":" + String(ESP.getChipId()) + ",\"wifi.macaddres\":\"" + WiFi.macAddress() + "\",\"error\":\"dht read failed\"}");
    return;
  }

  client.println("{\"esp.chipid\":" + String(ESP.getChipId()) + ",\"wifi.macaddres\":\"" + WiFi.macAddress() + "\",\"humidity\":" + String(h) + ",\"temperature\":" + String(t) + "}");
  Serial.println("{\"esp.chipid\":" + String(ESP.getChipId()) + ",\"wifi.macaddres\":\"" + WiFi.macAddress() + "\",\"humidity\":" + String(h) + ",\"temperature\":" + String(t) + "}");
  client.flush();
}


void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  delay(1000);

  setupWiFi();
  setupOTA();
  setupDht();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  loopOTA();
  loopLogstash();
  delay(2000);
}

