#include <ESP8266WiFi.h>
#include <PubSubClient.h>


const char* ssid     = "not-gonna-tell-ya";
const char* password = "nah-also-not-gonna-tell-ya-that";

const char* mqttServer = "homeassistant.local";
const int mqttPort = 1883;
const char* mqttUser = "mqtt";
const char* mqttPassword = "stop-looking-for-my-passwords";

uint8_t analogMoisturePin = A0;
uint8_t digitalMoisturePin = D2;

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
  Serial.begin(9600);

  pinMode(digitalMoisturePin, INPUT);

  float moistureValue = analogRead(analogMoisturePin);
  // 0 == wet, 1 == dry
  int isWet = !digitalRead(digitalMoisturePin);

  // Not really working as expected. Putting the sensor in water says ~60% O_o
  // float moisturePercentage = ( 100.00 - ( (sensorValue/1023.00) * 100.00 ) );

  // The Wi-FI library has auto-reocnnect. So we simply call it in setup() and there is no need to always check the conenction in loop()
  connectWifi();

  mqttClient.setServer(mqttServer, mqttPort);
  connectMqtt();

  char charMoistureValue[4];
  int intMoistureValue = moistureValue;
  char charIsWet[1];
  sprintf(charMoistureValue, "%d", intMoistureValue);
  sprintf(charIsWet, "%d", isWet);
  mqttClient.publish("home-assistant/plant-sensors/herbals/moisture", charMoistureValue);
  mqttClient.publish("home-assistant/plant-sensors/herbals/wet", charIsWet);
  
  Serial.println(moistureValue);
  Serial.println(isWet);

  delay(100);
  ESP.deepSleep(10*60e6); // Deep Sleep for 10 minutes
}

void loop() {
} 
