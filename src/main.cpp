#include "captive.h"
#include "password.h"
#include <Arduino.h>

const int baudRate = 115200;

void setup() {
  Serial.begin(baudRate);
  captiveSetup();
  setUpKeyboard(server);
}

void loop() {
  captiveLoop();
  delay(100);
}
