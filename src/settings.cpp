#include "settings.h"
#include "configuration.h"
#include "constants.h"
#include "crypto.h"
#include <Preferences.h>

static Preferences preferences;

void settingsSetup() {
    preferences.begin(F_NAMESPACE, false);
}

String getWifiPassword() {
    return preferences.getString("wifi_password", DEFAULT_WIFI_PASSWORD);
}

bool setWifiPassword(const char* password) {
    if (password == nullptr) {
        return false;
    }
    return preferences.putString("wifi_password", password);
}

String getMagicKey() {
    String magicKey = preferences.getString("magic_key", "");
    if (magicKey.length() == 0) {
        // If a passphrase is set, we can generate a magic key
        // but there is no direct way to check if a passphrase is set from here.
        // Assuming that if a passphrase is set, we can generate a key.
        // The check for the passphrase existence should be handled by the caller.
        uint8_t random_bytes[16];
        randomizeBuffer(random_bytes, sizeof(random_bytes));
        magicKey = String((char*)random_bytes, sizeof(random_bytes));
        preferences.putString("magic_key", magicKey);
    }
    return magicKey;
}

void factoryResetSettings() {
    preferences.clear();
}
