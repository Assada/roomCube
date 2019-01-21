#ifdef MASTER
#include <cube_esp.cpp>
#else
#include <cube.h>
#endif

void setup() {
  #ifdef DEBUG
    debug = true;
  #endif

  #ifdef MASTER
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
  #endif
  pinMode(MOVE_SENSOR, INPUT);
  digitalWrite (MOVE_SENSOR, LOW);

  Serial.begin(MSPEED);
  #ifdef BMP280
    if (!bme.begin(0x76)) {
      if (!bme.begin(0x76)) {
        errorObj["error"] = true;
        errorObj["code"] = "0x1"
        error = true;
      }
    }
  #endif
  ws2812fx.init();
  ws2812fx.setBrightness(0);
  ws2812fx.setSpeed(200);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
  #ifdef MASTER
    WiFi.begin(aSSID, aPASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      blinkLED(BLUE_LED, 100);
    }

    initTime();
    initServer();
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
  #ifdef MASTER
    if (last_conn_status != WiFi.status()) {
      if (WiFi.status() != WL_CONNECTED) {
        digitalWrite(BLUE_LED, 200);
          #ifdef DEBUG
            logDebug("WiFi connected");
          #endif
      } else {
        blinkLED(BLUE_LED, 200);
          #ifdef DEBUG
            logDebug("WiFi connected");
          #endif
      }
      last_conn_status = WiFi.status();
    }
  #endif
}

void slow() {
  #ifdef BMP280
    float temp = bme.readTemperature();
    float pressure = bme.readPressure() / 133.322;

     if (temp < -100 || temp >= 70) { //HACK
      error = true;
    }
  #endif

  if (error) {
    #ifdef BMP280
      error = false;
      if (!bme.begin(0x76)) {
        errorObj["error"] = true;
        errorObj["code"] = "0x2"
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
    #ifdef BMP280
    obj["temp"] = temp;
    obj["pressure"] = pressure;
    obj["mt"] = (tempDHT+temp)/2;
    #endif

    obj["td"] = dht.readTemperature();
    obj["h"] = dht.readHumidity();
    obj["m"] = moveSensor;
    obj["i"] = illumination;
  }

  #ifdef MASTER
    if (error) {
      blinkLED(BLUE_LED, 100);
      blinkLED(GREEN_LED, 600);
    }
  #endif
}

void processLight() {
  ws2812fx.service();
}

void speed() {
  processLight();
  
  illumination = analogRead(LDR_PIN);
  moveSensor = digitalRead(MOVE_SENSOR);

  if ((millis() - lastMotion > 2000) && moveSensor == 1 && illumination <= LIGHT_LIMIT) {
    lightOn(255, 0, 255, 0);
    start = millis();
  }

  if (lightStatus && (millis() - start > LIGHT_TIMER || illumination > LIGHT_LIMIT)) {
    start = LIGHT_TIMER + 1;
    lightOff();
    lastMotion = millis();
  }
}

void lightOn(int r, int g, int b, int m) {
  if(!lightStatus) {
    ws2812fx.setSegment(0,  0,  8, FX_MODE_STATIC, 0x007BFF, 200, FADE_SLOW);
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
