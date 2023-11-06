#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

const char* ssid = "Artego";
const char* password = "1111111111";

const int ledPin = 13;
const int potens = 36;
const int PIN_STEP = 23;
const int PIN_DIR = 22;
const int PIN_EN = 21;

AsyncWebServer server(80);

String potentiometerValue = "0";
bool isMotorOn = false;
bool isClockwise = true;
int currentSpeed = 100;

String processor(const String& var) {
  if (var == "STATE") {
    if (digitalRead(ledPin)) {
      return "ON";
    } else {
      return "OFF";
    }
  } else if (var == "POTENTIOMETER") {
    return potentiometerValue;
  }
  return String();
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(potens, INPUT);
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_EN, OUTPUT);

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  digitalWrite(PIN_EN, HIGH);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/led", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("is_on")) {
      String temp = request->getParam("is_on")->value();
      if(temp == "true")
      {
        digitalWrite(ledPin, LOW); 
        request->send(200, "text/plain", "On");
      }
      else if(temp == "false")
      {
        digitalWrite(ledPin, HIGH);  
        request->send(200, "text/plain", "Off");
      }
    }
  });

  server.on("/getPotentiometer", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", potentiometerValue);
  });

  server.on("/rotate", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("clockwise")) {
      String temp = request->getParam("clockwise")->value();
      isClockwise = temp == "true";
    }
    isMotorOn = true;
    digitalWrite(PIN_EN, LOW); // Включаем мотор
    String str_to_send = "motor started " + String(isClockwise ? "clockwise" : "counter clockwise");
    request->send(200, "text/plain", str_to_send);
  });

  server.on("/stopRotation", HTTP_GET, [](AsyncWebServerRequest *request) {
    isMotorOn = false;
    digitalWrite(PIN_EN, HIGH); // Выключаем мотор
    request->send(200, "text/plain", "motor stopped");
  });

  server.begin();
}

void loop() {
  int analogValue = analogRead(potens);
  potentiometerValue = String(analogValue);
  if (isMotorOn) {
    digitalWrite(PIN_DIR, isClockwise ? HIGH : LOW);
    digitalWrite(PIN_STEP, HIGH);
    delay(currentSpeed);
    digitalWrite(PIN_STEP, LOW);
    delay(currentSpeed);
  }
}
