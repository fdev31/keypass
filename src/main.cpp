#include "captive.h"
#include "password.h"
#include <Arduino.h>

const int baudRate = 9600;
const unsigned long SHUTDOWN_TIMEOUT = 5 * 60 * 1000; // 5 minutes in ms
unsigned long lastClientTime;

void setup() {
  Serial.begin(baudRate);
  captiveSetup();
  setUpKeyboard(server);
  lastClientTime = millis();
}

void loop() {
  captiveLoop();

  // Check if it's time to go to deep sleep
  if (millis() - lastClientTime >= SHUTDOWN_TIMEOUT) {
    ESP.deepSleep(0); // Deep sleep indefinitely
  }

  delay(200);
}
