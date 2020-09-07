#include <ESP8266WiFi.h>
// Using https://pubsubclient.knolleary.net/
#include <PubSubClient.h>
#include <SimpleTimer.h>
// Using https://github.com/mcxiaoke/ESPDateTime
#include <DateTime.h>


const char* ssid     = "not-gonna-tell-ya";
const char* password = "nah-also-not-gonna-tell-ya-that";

const char* mqttServer = "homeassistant.local";
const int mqttPort = 1883;
const char* mqttUser = "mqtt";
const char* mqttPassword = "stop-looking-for-my-passwords";

const byte bellPin = 4;

const byte maxConnTries = 15;
volatile boolean doorbellRang = false;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
SimpleTimer timer;


void resetTrigger() {
  doorbellRang = false;

  mqttClient.publish("home-assistant/doorbell/state", "off");
}

void resetLED() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void mqttAvailability() {
  mqttClient.publish("home-assistant/doorbell/availability", "online");
  String strDateTime = DateTime.format("%Y-%m-%dT%H:%M:%S%z");
  char charDateTime[strDateTime.length()];
  strDateTime.toCharArray(charDateTime, strDateTime.length());
  mqttClient.publish("home-assistant/doorbell/lastUpdated", charDateTime);

  timer.setTimeout(60000, mqttAvailability);
}

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
  pinMode(bellPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  resetLED();

  Serial.begin(9600);

  // The Wi-FI library has auto-reconnect. So we simply call it in setup() and there is no need to always check the conenction in loop()
  connectWifi();

  mqttClient.setServer(mqttServer, mqttPort);
  connectMqtt();

  DateTime.begin();

  mqttAvailability();
}

void loop() {
  connectMqtt();

  if (digitalRead(bellPin) == LOW && !doorbellRang) {
    doorbellRang = true;

    Serial.println("Ding Dong!");
    mqttClient.publish("home-assistant/doorbell/state", "on");
    
    digitalWrite(LED_BUILTIN, LOW);

    timer.setTimeout(5000, resetTrigger);
    timer.setTimeout(3000, resetLED);
  }

  timer.run();
