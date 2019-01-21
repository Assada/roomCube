#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#ifdef BMP280
    #include <Adafruit_BMP280.h>
#endif

#include <DHT.h>
#include <WS2812FX.h>

bool debug            = false;

int illumination      = 0;
int moveSensor        = 0;

#define LDR_PIN       A0
#define DHT_PIN       4
#define CIRCLE_PIN    14
#define MOVE_SENSOR   5

#define LIGHT_LIMIT   100
#define LIGHT_TIMER   4000

#define DELAY_TIME    1
#define RESPONSE_TIME 2000

int tick              = 0;
int ticks             = RESPONSE_TIME / DELAY_TIME;

bool error            = false;
int errors            = 0;

bool lightStatus      = false;

unsigned long start = 0;
unsigned long lastMotion = 0;

#ifdef BMP280
Adafruit_BMP280 bme;
const int capacity    = JSON_OBJECT_SIZE(7);
#else
const int capacity    = JSON_OBJECT_SIZE(4);
#endif

StaticJsonBuffer <capacity> jb;
JsonObject &obj = jb.createObject();


const int errorCapacity = JSON_OBJECT_SIZE(2);

StaticJsonBuffer <errorCapacity> jbe;
JsonObject &errorObj = jbe.createObject();


WS2812FX ws2812fx = WS2812FX(8, CIRCLE_PIN, NEO_RGB + NEO_KHZ800);
DHT dht(DHT_PIN, DHT11);

struct Color {
  int r;
  int g;
  int b;
};

struct Config {
  bool debug;
  int lightLimit;
  int lightTimer;
  int reactionInterval;
  int errorsToRestart;
  Color color;
};

void slow();
void speed();
void lightOn(int r, int g, int b, int m);
void lightOff();

#ifdef DEBUG
void logDebug(String value) {
  if (debug) {
    Serial.println(value);
  }
}
#endif