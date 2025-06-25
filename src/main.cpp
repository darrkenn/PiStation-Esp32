
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "wifiCreds.h"
#include "DHT.h"
#include "ArduinoJson.h"
#include <WiFiManager.h>
#include <Adafruit_BMP280.h>

#define DHT_PIN 4
#define WATER_PIN 36


const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

StaticJsonDocument<1024> jsonDoc;
char buffer[1024];

unsigned long callDelay = 3000;
unsigned long lastCall;

int temp;
int humid;
int rain;
float pressurePA;
float pressureHPA;
float airPressure;
String isRaining;

enum IsRaining {
    No,
    Yes
};

IPAddress staticIP(WIFI_IP);
IPAddress gateway(WIFI_GATEWAY);
IPAddress subnet(WIFI_SUBNET);


//Initialisation
WebServer server(80);
DHT dht(DHT_PIN,DHT22);
Adafruit_BMP280 bmp;


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

IsRaining calculateRainLevel(int rain) {
    if (rain <= 300) {
        return No;
    }
    return Yes;
}

String getRainLevel() {
    while (true) {
        delay(500);
        rain = analogRead(WATER_PIN);
        if (isnan(rain)) {
            Serial.println("Failed to read rain from analog");
        }
        const IsRaining level = calculateRainLevel(rain);
        if (level == Yes) {
            return "It's raining";
        }
            return "Not raining";
    }
}

int bmpReadPressure() {
    while (true) {
        if (bmp.takeForcedMeasurement()) {
            pressurePA = bmp.readPressure();
            pressureHPA = pressurePA * 0.01;
            return pressureHPA;
        }
        Serial.println("Failed to take forced measurement");
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
    isRaining = getRainLevel();
    airPressure = bmpReadPressure();

    Serial.println("getJsonValues");
    jsonDoc.clear();
    jsonDoc["temperature"] = temp;
    jsonDoc["humidity"] = humid;
    jsonDoc["rain"] = isRaining;
    jsonDoc["airPressure"] = airPressure;
    serializeJson(jsonDoc, buffer);
    server.send(200, "application/json", buffer);
    delay(10000);
    Serial.println("Sleeping...");
    esp_sleep_enable_timer_wakeup(9 * 60 * 1000000ULL);
    esp_deep_sleep_start();
}




void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Waking up!");
    dht.begin();
    Serial.println("DHT WORKING!");

    analogSetAttenuation(ADC_11db);

    if (!bmp.begin()) {
        Serial.println("Failed to initialize BMP280 check wiring or use a different address");
        while (1) delay(10);
    }

    bmp.setSampling(
        Adafruit_BMP280::MODE_FORCED,
        Adafruit_BMP280::SAMPLING_X2,
        Adafruit_BMP280::SAMPLING_X16,
        Adafruit_BMP280::FILTER_X16,
        Adafruit_BMP280::STANDBY_MS_500
);

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
    Serial.println(WiFi.localIP());

    server.on("/", getJsonValues);
    server.begin();
    Serial.println("Successfully initialized");
}




void loop() {
    server.handleClient();
    if (bmp.takeForcedMeasurement()) {
        // can now print out the new measurements
        Serial.print(F("Temperature = "));
        Serial.print(bmp.readTemperature());
        Serial.println(" *C");

        Serial.print(F("Pressure = "));
        Serial.print(bmp.readPressure());
        Serial.println(" Pa");

        Serial.print(F("Approx altitude = "));
        Serial.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
        Serial.println(" m");

        Serial.println();
        delay(2000);
    } else {
        Serial.println("Forced measurement failed!");
    }
}



