#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>

//const char *ssid      = SSID;
//const char *password  = PASSWORD;

WiFiServer server(80);

void blinkLED(int pin, int d) {
  digitalWrite(pin, LOW);
  digitalWrite(pin, HIGH);
  delay(d);
  digitalWrite(pin, LOW);
}