#include <ESP8266WiFi.h>
// #include <ESP8266WebServer.h>
#include <TimeLib.h>
#include <Wire.h>
#include <SPI.h>
#include <Timezone.h>
#include <EasyNTPClient.h>
#include <WiFiUdp.h>


#define RED_LED       15
#define GREEN_LED     12
#define BLUE_LED      13

int last_conn_status;

// ESP8266WebServer server(80);
WiFiUDP udp;
EasyNTPClient ntpClient(udp, "pool.ntp.org");

TimeChangeRule *tcr;
TimeChangeRule fiEET = {"EET", Last, Sun, Oct, 3, 120}; // Standard time
TimeChangeRule fiEEST = {"EEST", Last, Sun, Mar, 3, 180}; // Summertime
Timezone tzFI(fiEET, fiEEST);

void blinkLED(int pin, int d);
void initTime();
void dataAction();
void initServer();