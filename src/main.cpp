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


void createJson (const char *name,const int value) {
    jsonDoc.clear();
    jsonDoc["name"] = name;
    jsonDoc["value"] = value;
    serializeJson(jsonDoc, buffer);
}

void addJsonValue (const char *name, const int value) {
    JsonObject obj = jsonDoc.createNestedObject();
    obj["name"] = name;
    obj["value"] = value;
}

void getJsonValues() {
    humid = dht.readHumidity();
    temp = dht.readTemperature();
    Serial.println("getJsonValues");
    jsonDoc.clear();
    addJsonValue("temperature", temp);
    addJsonValue("humidity", humid);
    serializeJson(jsonDoc, buffer);
    server.send(200, "application/json", buffer);
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
    Serial.println("Connected Successfully!");
    Serial.println("IP: ");
    Serial.println(WiFi.localIP());



    server.on("/", getJsonValues);
    server.begin();
    Serial.println("Successfully initialized");
}




void loop() {
    server.handleClient();


    Serial.println(server.client().localIP());
    if (millis() > lastCall + callDelay ) {
        lastCall = millis();
        temp = dht.readTemperature();
        humid = dhtReadHumidity();

    }

    delay(2000);
}



