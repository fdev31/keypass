#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

void settingsSetup();
String getWifiPassword();
bool setWifiPassword(const char* password);
String getMagicKey();
void factoryResetSettings();

#endif // SETTINGS_H
