#include "configuration.h"

#include "captive.h"
#include "password.h"
#include <Arduino.h>

#ifdef ENABLE_GRAPHICS
#include "graphics.h"
#endif

const int baudRate = 9600;
unsigned long lastClientTime;

void setup() {
#ifdef ENABLE_GRAPHICS
  graphicsSetup();
#endif
  Serial.begin(baudRate);
  captiveSetup();
  setUpKeyboard(server);
  setUpPassword();
  lastClientTime = millis();
}

void loop() {
  captiveLoop();
#ifdef ENABLE_GRAPHICS
  graphicsLoop();
#endif
  delay(100);

  // Check if it's time to go to deep sleep
  if (millis() - lastClientTime >= SHUTDOWN_TIMEOUT) {
    Serial.flush(); // Make sure serial output is complete before sleep
    shutdownGraphics();
    esp_deep_sleep_start();
  }
}
