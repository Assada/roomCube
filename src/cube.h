#include <Arduino.h>
#include <Adafruit_Sensor.h>
#ifdef BMP280
    #include <Adafruit_BMP280.h>
#endif
#include <ArduinoJson.h>
#include <DHT.h>
#include <WS2812FX.h>

bool debug            = false;

int illumination      = 0;
int moveSensor        = 0;
float humi            = 0.0;
float tempDHT         = 0.0;

String response       = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"error\": false,\"code\": 0}\n";

#define LDR_PIN       A0
#define DHT_PIN       7
#define CIRCLE_PIN    8

#define MOVE_SENSOR   2
#define LIGHT_LIMIT   200
#define LIGHT_TIMER   200000
#define TIMER_MS      2000

#define DELAY_TIME    1
#define RESPONSE_TIME 2000

int tick              = 0;
int ticks             = RESPONSE_TIME / DELAY_TIME;

bool error            = false;
int errors            = 0;

bool lightStatus      = false;
unsigned long last_change = 0;
unsigned long now = 0;
unsigned long start = 0;

#ifdef BMP280
Adafruit_BMP280 bme;
const int capacity    = JSON_OBJECT_SIZE(7);
#else
const int capacity    = JSON_OBJECT_SIZE(5);
#endif
WS2812FX ws2812fx = WS2812FX(8, CIRCLE_PIN, NEO_RGB + NEO_KHZ800);
DHT dht(DHT_PIN, DHT11);


#ifdef DEBUG
void logDebug(String value) {
  if (debug) {
    Serial.println(value);
  }
}
#endif