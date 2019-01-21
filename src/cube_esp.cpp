#include <cube.h>
#include <cube_esp.h>

void blinkLED(int pin, int d) {
  digitalWrite(pin, LOW);
  digitalWrite(pin, HIGH); 
  delay(d);
  digitalWrite(pin, LOW);
}

void initTime() {
  time_t utc = NULL;
  while (!utc) {
    utc = ntpClient.getUnixTime();
  }
  
  time_t t = tzFI.toLocal(utc);
  setTime(t);
}

void dataAction() {
    if(!error) {
        char json[jb.size()];
        obj.printTo(json, sizeof(json));
        server.send(200, "text/json", json);
    } else {
        char json[jbe.size()];
        errorObj.printTo(json, sizeof(json));
        server.send(500, "text/json", json);
    }
}

void initServer() {
  server.begin();
  server.on("/api/data", dataAction);
}