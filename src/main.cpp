#include "captive.h"
#include "password.h"
#include <Arduino.h>

void setup() {
  captiveSetup();
  setUpKeyboard(server);
}

void loop() {
  captiveLoop();
  delay(100);
}
