#include "configuration.h"

#include "captive.h"
#include "password.h"
#include <Arduino.h>

#ifdef ENABLE_GRAPHICS
#include "graphics.h"
#endif

const int baudRate = 9600;
const unsigned long SHUTDOWN_TIMEOUT = 5 * 60 * 1000; // 5 minutes in ms
unsigned long lastClientTime;

void setup() {
  Serial.begin(baudRate);
  captiveSetup();
#ifdef ENABLE_GRAPHICS
  graphicsSetup();
#endif
  setUpKeyboard(server);
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
    ESP.deepSleep(0); // Deep sleep indefinitely
  }
}
