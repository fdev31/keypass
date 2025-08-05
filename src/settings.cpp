#include "settings.h"
#include "configuration.h"
#include "constants.h"
#include "crypto.h"
#include <Preferences.h>

static Preferences preferences;

void settingsSetup() {
  // No longer needed as each function handles begin/end.
}

String getWifiPassword() {
  preferences.begin(F_NAMESPACE, true);
  String password =
      preferences.getString(WIFI_PASSWORD_KEY, DEFAULT_WIFI_PASSWORD);
  preferences.end();
  return password;
}

bool setWifiPassword(const char *password) {
  if (password == nullptr) {
    return false;
  }
  preferences.begin(F_NAMESPACE, false);
  bool success = preferences.putString(WIFI_PASSWORD_KEY, password);
  preferences.end();
  return success;
}

String getWifiSSID() {
  preferences.begin(F_NAMESPACE, true);
  String ssid = preferences.getString(DEVICE_NAME_KEY, DEFAULT_WIFI_SSID);
  preferences.end();
  return ssid;
}

bool setWifiSSID(const char *ssid) {
  if (ssid == nullptr) {
    return false;
  }
  preferences.begin(F_NAMESPACE, false);
  bool success = preferences.putString(DEVICE_NAME_KEY, ssid);
  preferences.end();
  return success;
}

String getMagicKey() {
  preferences.begin(F_NAMESPACE, false);
  String magicKey = preferences.getString(MAGIC_KEY_KEY, "");
  if (magicKey.length() == 0) {
    // If a passphrase is set, we can generate a magic key
    // but there is no direct way to check if a passphrase is set from here.
    // Assuming that if a passphrase is set, we can generate a key.
    // The check for the passphrase existence should be handled by the caller.
    uint8_t random_bytes[16];
    randomizeBuffer(random_bytes, sizeof(random_bytes));
    magicKey = String((char *)random_bytes, sizeof(random_bytes));
    preferences.putString(MAGIC_KEY_KEY, magicKey);
  }
  preferences.end();
  return magicKey;
}

void factoryResetSettings() {
  preferences.begin(F_NAMESPACE, false);
  preferences.clear();
  preferences.end();
}
