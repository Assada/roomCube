#include <cube.h>
#ifdef MASTER
#include <cube_esp.h>
#endif

void setup() {
  #ifdef DEBUG
    debug = true;
  #endif

  pinMode(MOVE_SENSOR, INPUT);
  digitalWrite (MOVE_SENSOR, LOW);

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
  ws2812fx.setBrightness(0);
  ws2812fx.setSpeed(200);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
  #ifdef MASTER
    Serial.println("Try to connect");
    WiFi.begin(aSSID, aPASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      blinkLED(12, 100);
      Serial.print(".");
    }
    Serial.println("Connected");

    server.begin();
  #endif
}

void blinkLED(int pin, int d) {
  digitalWrite(pin, LOW);
  digitalWrite(pin, HIGH);
  delay(d);
  digitalWrite(pin, LOW);
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
    json = "";
    StaticJsonBuffer <capacity> jb;
    JsonObject &obj = jb.createObject();

    #ifdef BMP280
    obj["temp"] = temp;
    obj["pressure"] = pressure;
    obj["mt"] = (tempDHT+temp)/2;
    #endif

    obj["td"] = dht.readTemperature();
    obj["h"] = dht.readHumidity();
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
}

void speed() {
  processLight();
  
  illumination = analogRead(LDR_PIN);
  moveSensor = digitalRead(MOVE_SENSOR);

  if (moveSensor == 1 && illumination <= LIGHT_LIMIT) {
    lightOn(255, 0, 255, 0);
    start = millis();
  }

  if (moveSensor == 0 && lightStatus && (millis() - start > LIGHT_TIMER || illumination > LIGHT_LIMIT)) {
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
    Serial.println(F("RSPONSE!"));
    delay(1);
  #endif
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
