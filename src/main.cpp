#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "wifiCreds.h"
#include "DHT.h"
#include "ArduinoJson.h"
#include <WiFiManager.h>

#define DHT_PIN 4

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

StaticJsonDocument<1024> jsonDoc;
char buffer[1024];

unsigned long callDelay = 3000;
unsigned long lastCall;

int temp;
int humid;

IPAddress staticIP(WIFI_IP);
IPAddress gateway(WIFI_GATEWAY);
IPAddress subnet(WIFI_SUBNET);


//Initialisation
WebServer server(80);
DHT dht(DHT_PIN,DHT22);

float dhtReadTemperature() {
    while (true) {
        delay(500);
        const float temperature = dht.readTemperature();

        if (isnan(temperature)) {
            Serial.println("Failed to read temperature from DHT sensor");
        }
        else {
            return std::round(temperature);
        }
    }
}

float dhtReadHumidity() {
    while (true) {
        delay(500);
        float humidity = dht.readHumidity();
        if (isnan(humidity)) {
            Serial.println("Failed to read humidityfrom DHT sensor");
        }
        else {
            return std::round(humidity);
        }
    }
}


void createJson (const int value) {
    jsonDoc.clear();
    jsonDoc["value"] = value;
    serializeJson(jsonDoc, buffer);
}

void getJsonValues() {
    humid = dht.readHumidity();
    temp = dht.readTemperature();
    Serial.println("getJsonValues");
    jsonDoc.clear();
    jsonDoc["temperature"] = temp;
    jsonDoc["humidity"] = humid;
    serializeJson(jsonDoc, buffer);
    server.send(200, "application/json", buffer);
    delay(1000);
    Serial.println("Sleeping...");
    esp_sleep_enable_timer_wakeup(9 * 60 * 1000000ULL);
    esp_deep_sleep_start();
}



void setup() {
    Serial.begin(115200);
    delay(1000);

    dht.begin();
    Serial.println("DHT WORKING!");

    Serial.println("Connecting to WiFi");
    WiFi.begin(ssid, password);
    WiFi.mode(WIFI_MODE_STA);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    if(!WiFi.config(staticIP, gateway, subnet)) {
        Serial.println("Failed to configure Static IP");
    } else {
        Serial.println("Static IP configured!!");
    }

    Serial.println("Connected Successfully!");
    Serial.println("IP: ");
    Serial.println(WiFi.localIP());\



    server.on("/", getJsonValues);
    server.begin();
    Serial.println("Successfully initialized");
}




void loop() {
    server.handleClient();
}



