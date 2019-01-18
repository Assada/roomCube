#include <cube.h>
#ifdef MASTER
#include <cube_esp.h>
#endif

void setup() {
  #ifdef DEBUG
    debug = true;
  #endif

  Serial.begin(MSPEED);
  #ifdef BMP280
    if (!bme.begin(0x76)) {
      if (!bme.begin(0x76)) {
        response = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: application/json\r\n\r\n{\"error\": true,\"code\": \"0001\"}\n";
        error = true;
      }
    }
  #endif
  ws2812fx.init();
  ws2812fx.setBrightness(255);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
  #ifdef MASTER
    WiFi.begin(aSSID, aPASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      blinkLED(13, 100);
    }

    server.begin();
  #endif
}

void loop() {
  speed();
  delay(DELAY_TIME);
  tick = 1 + tick;
  if (tick >= ticks) {
    slow();
    tick = 0;
  }
}

void slow() {
  String json = "{}";
  char buffer[255];
  #ifdef BMP280
    float temp = bme.readTemperature();
    float pressure = bme.readPressure() / 133.322;

     if (temp < -100 || temp >= 70) { //HACK
      error = true;
    }
  #endif

  humi = dht.readHumidity();
  tempDHT = dht.readTemperature();

  if (error) {
    #ifdef BMP280
      error = false;
      if (!bme.begin(0x76)) {
        response = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: application/json\r\n\r\n{\"error\": true,\"code\": \"0002\"}\n";
        error = true;
        errors = 1 + errors;
      }
    #endif
  }
  #ifdef MASTER
    if (errors > 500) {
      ESP.restart();
      errors = 0;
    }
  #endif
  if (!error) {
    float midTemp = 0.0;
    #ifdef BMP280
      midTemp = (tempDHT+temp)/2;
    #endif

    json = "";
    StaticJsonBuffer <capacity> jb;
    JsonObject &obj = jb.createObject();

    #ifdef BMP280
    obj["temp"] = temp;
    obj["pressure"] = pressure;
    #endif

    obj["td"] = tempDHT;
    obj["mt"] = midTemp;
    obj["h"] = humi;
    obj["m"] = moveSensor;
    obj["i"] = illumination;
    obj.printTo(json);
    sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n%s\n", json.c_str());
    response = buffer;
  }

  #ifdef MASTER
    if (error) {
      blinkLED(13, 100);
      blinkLED(12, 600);
    }
  #endif
  #ifdef DEBUG
    Serial.println(response);
  #endif
}

void processLight() {
  now = millis();

  ws2812fx.service();

  if(now - last_change > TIMER_MS) {
    ws2812fx.setMode((ws2812fx.getMode() + 1) % ws2812fx.getModeCount());
    last_change = now;
  }
}

void speed() {
  processLight();
  
  illumination = 1024 - analogRead(LDR_PIN);
  moveSensor = digitalRead(MOVE_SENSOR);

  if (moveSensor == 1 && illumination <= LIGHT_LIMIT) {
    lightOn(255, 0, 255, 0);
    start = millis();
  }

  if (lightStatus && (millis() - start > LIGHT_TIMER || illumination > LIGHT_LIMIT)) {
    start = LIGHT_TIMER + 1;
    lightOff();
  }

  #ifdef MASTER
    WiFiClient client = server.available();
    if (!client) {
      return;
    }
    while (!client.available()) {
      delay(1);
    }

    client.flush();
    client.print(response);
    delay(1);
  #endif
}

void lightOn(int r, int g, int b, int m) {
  if(!lightStatus) {
    ws2812fx.setBrightness(255);
    lightStatus = true;
    #ifdef DEBUG
      logDebug(F("Light: ON"));
    #endif
  }
}

void lightOff() { 
  ws2812fx.setBrightness(0);
  lightStatus = false;
  #ifdef DEBUG
    logDebug(F("Light: OFF"));
  #endif
}
