#include <ESP8266WiFi.h>
// Using https://pubsubclient.knolleary.net/
#include <PubSubClient.h>
// Using https://github.com/mcxiaoke/ESPDateTime
#include <DateTime.h>


const char* ssid     = "not-gonna-tell-ya";
const char* password = "nah-also-not-gonna-tell-ya-that";

const char* mqttServer = "homeassistant.local";
const int mqttPort = 1883;
const char* mqttUser = "mqtt";
const char* mqttPassword = "stop-looking-for-my-passwords";

const byte analogMoisturePin = A0;
const byte enableSensorPin = D2;

// Measured myself, using this guide: https://media.digikey.com/pdf/Data%20Sheets/DFRobot%20PDFs/SEN0193_Web.pdf
const int airValue = 910;
const int waterValue = 412;
// The original script devides into "Very wet", "Wet" and "Dry". But we will only care about "Dry"
// int intervals = (airValue - waterValue) / 3;
const int plantDryThreshold = ((airValue - waterValue) * 0.6) + waterValue;

const byte maxConnTries = 15;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


void connectWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    int wifiConnTries = 0;

    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && wifiConnTries < maxConnTries) {
      wifiConnTries++;
      Serial.printf("Connecting to Wi-Fi %s ...\n", ssid);
      delay(1000);
    }

    if (WiFi.status() != WL_CONNECTED && wifiConnTries >= maxConnTries) {
      Serial.println("Maximum number of Wi-Fi connection attempts reached. Rebooting...");
      ESP.restart();
    }

    Serial.printf("Connected to Wi-Fi! IP address: %s\n", WiFi.localIP().toString().c_str());
  }
}

void connectMqtt() {
  if (!mqttClient.connected()) {
    int mqttConnTries = 0;
    while (!mqttClient.connected() && mqttConnTries < maxConnTries) {
      mqttConnTries++;
      Serial.printf("Connecting to MQTT server at %s on port %d ...\n", mqttServer, mqttPort);

      if (!mqttClient.connect("esp8266-doorbell", mqttUser, mqttPassword )) {
        Serial.print("Failed with state ");
        Serial.println(mqttClient.state());
        delay(1000);
      }
    }

    if (!mqttClient.connected() && mqttConnTries >= maxConnTries) {
      Serial.println("Maximum number of MQTT connection attempts reached. Rebooting...");
      ESP.restart();
    }

    Serial.println("Connected to MQTT server!");
  }
}

void setup() {
  pinMode(enableSensorPin, OUTPUT);
  digitalWrite(enableSensorPin, HIGH);

  Serial.begin(9600);

  // The Wi-FI library has auto-reocnnect. So we simply call it in setup() and there is no need to always check the conenction in loop()
  connectWifi();

  mqttClient.setServer(mqttServer, mqttPort);
  connectMqtt();

  float soilMoistureValue = analogRead(analogMoisturePin);
  Serial.println(soilMoistureValue);

  char charMoistureValue[4];
  int intMoistureValue = soilMoistureValue;
  sprintf(charMoistureValue, "%d", intMoistureValue);
  mqttClient.publish("home-assistant/plant-sensors/herbals/moisture", charMoistureValue);

  int isWet = 1;
  if(soilMoistureValue > plantDryThreshold) {
    isWet = 0;
  }
  Serial.printf("Is wet: %d\n", isWet);

  char charIsWet[1];
  sprintf(charIsWet, "%d", isWet);
  mqttClient.publish("home-assistant/plant-sensors/herbals/wet", charIsWet);

  DateTime.begin();
  if (!DateTime.isTimeValid()) {
    Serial.println("Failed to get time from server.");
  }
  String strDateTime = DateTime.format("%Y-%m-%dT%H:%M:%S%z");
  char charDateTime[strDateTime.length()];
  strDateTime.toCharArray(charDateTime, strDateTime.length());
  mqttClient.publish("home-assistant/plant-sensors/herbals/lastUpdated", charDateTime);

  digitalWrite(enableSensorPin, LOW);

  delay(100);
  ESP.deepSleep(10*60e6); // Deep Sleep for 10 minutes
}

void loop() {
} 
