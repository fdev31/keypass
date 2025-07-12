#include "password.h"
#include "Preferences.h"
#include "configuration.h"
#include "constants.h"
#include "crypto.h"
#include "graphics.h"
#include "hid.h"
#include "importexport.h"
#include "indexPage.h"
#include "main.h"
#include "restore.h"
#include "utils.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <StreamString.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Core business logic functions (independent of AsyncWebServerRequest)

Password password;

static Password readPassword(int id) {
#if USE_EEPROM_API
  Password password;
  int address = id * sizeof(Password);
  EEPROM.get(address, password);
  // Add validation check to ensure data is valid
  if (password.name[0] == 0xFF) { // EEPROM default state is 0xFF
    password.name[0] = '\0';      // Mark as empty
  }
  return password;
#else
  static uint8_t passBuffer[MAX_PASS_LEN];
  Preferences preferences;
  const char *key = mkEntryName(id);
  preferences.begin(key, true);
  size_t pass_version = preferences.getInt(F_FORMAT, 0);
  strlcpy(password.name, preferences.getString(F_NAME).c_str(),
          MAX_NAME_LEN - 1);
  preferences.getBytes(F_PASSWORD, passBuffer, MAX_PASS_LEN);
  password.layout = preferences.getInt(F_LAYOUT, -1);
  preferences.end();

  if (pass_version == 0) { // unencrypted version 0
    strlcpy((char *)password.password, (char *)passBuffer, MAX_PASS_LEN - 1);
  } else { // one version only
    decryptPassword((uint8_t *)passBuffer, (char *)password.password, id,
                    MAX_PASS_LEN);
  }
  return password;
#endif
}

static void clearPassword(int id) {
  Preferences preferences;
  preferences.begin(mkEntryName(id), false);
  preferences.clear();
  preferences.end();
}

static void writePassword(int id, const Password &password) {
#if USE_EEPROM_API
  int address = id * sizeof(Password);
  EEPROM.put(address, password);
  EEPROM.commit();
#else
  Preferences preferences;
  preferences.begin(mkEntryName(id), false);
  preferences.putString(F_NAME, password.name);
  static uint8_t encrypted_password[MAX_PASS_LEN];
  int pass_len = strlen((const char *)password.password);
  randomizeBuffer((uint8_t *)password.password + pass_len + 1,
                  MAX_PASS_LEN - pass_len - 1);
  encryptBuffer((char *)password.password, encrypted_password, id,
                MAX_PASS_LEN);
  preferences.putBytes(F_PASSWORD, encrypted_password, MAX_PASS_LEN);
  preferences.putInt(F_FORMAT, FORMAT_VERSION);
  preferences.putInt(F_LAYOUT, password.layout);
  preferences.end();
#endif
}

static void sendAnyKeymap(const char *text, int layout, int newline) {
  if (layout == -1) {
    while (*text) {
      sendKey(*text++, true);
    }

  } else {
    while (*text) {
      sendKeymap(*text++, layout);
    }
  }
  if (newline) {
    sendKey('\n', false); // Send newline if requested
  }
}

bool typeRawText(const char *text, int layout, bool sendNewline) {
  if (text == nullptr)
    return false;

  ping();
  sendAnyKeymap(text, layout, sendNewline);
  return true;
}

bool typePassword(int id, int layout, bool sendNewline) {
  if (id < 0 || id >= MAX_PASSWORDS)
    return false;

  ping();
#ifdef ENABLE_GRAPHICS
  printText(1, "Typing");
#endif

  password = readPassword(id);
  char *text = (char *)password.password;

  // If layout is specified, use it; otherwise use the stored layout
  int effectiveLayout = (layout != -1) ? layout : password.layout;

  sendAnyKeymap(text, effectiveLayout, sendNewline);
  return true;
}

const char *fetchPassword(int id) {
  if (id < 0 || id >= MAX_PASSWORDS)
    return nullptr;

  ping();
  password = readPassword(id);
  return (char *)password.password;
}

bool editPassword(int id, const char *name, const char *clear_pass,
                  int layout) {
  if (id < 0 || id >= MAX_PASSWORDS)
    return false;

  ping();

  password = readPassword(id);

  // Update fields if provided
  if (layout != -2) { // -2 means "don't change"
    password.layout = layout;
  }

  if (name != nullptr) {
    strlcpy(password.name, name, MAX_NAME_LEN - 1);
  }

  if (clear_pass != nullptr) {
    strlcpy((char *)password.password, clear_pass, MAX_PASS_LEN - 1);
  }

  // Save the updated password
  writePassword(id, password);
  return true;
}

String listPasswords() {
  Password pwd;
  ping();

  // Create a dynamic JSON string to hold the list of passwords
  String json = "{\"passwords\":[";

  // Loop through password ids and add existing passwords to the JSON
  bool firstItem = true;
  for (int id = 0; id < MAX_PASSWORDS; id++) {
    pwd = readPassword(id);
    if (pwd.name[0] == '\0')
      break;

    if (!firstItem) {
      json += ",";
    }
    json += "{\"name\":\"" + String(pwd.name) + "\",\"uid\":" + String(id) +
            ",\"layout\":" + String(pwd.layout) +
            ",\"len\":" + strlen((char *)pwd.password) + "}";
    firstItem = false;
  }

  Preferences prefs;
  size_t entries_left = prefs.freeEntries();
  json += String("], \"free\":") + entries_left + "}";

  return json;
}

void factoryReset() {
  ping();
#ifdef ENABLE_GRAPHICS
  printText(1, "Reset");
#endif
  Preferences preferences;

  for (int id = 0; id < MAX_PASSWORDS; id++) {
    clearPassword(id);
  }

#if not USE_EEPROM_API
  preferences.begin(F_NAMESPACE, false);
  preferences.clear(); // Clear all preferences
  preferences.end();
#elif
#pragma message "EEPROM API is used, factory reset not implemented for settings"
#endif
}

bool setWifiPassword(const char *pass) {
  if (pass == nullptr)
    return false;

  ping();
  Preferences preferences;
  preferences.begin(F_NAMESPACE, false);
  preferences.putString("wifi_password", pass);
  preferences.end();
  return true;
}

bool setupPassphrase(const char *phrase) {
  if (phrase == nullptr)
    return false;
  ping();
  return setPassPhrase(phrase);
}

String dumpPasswords() {
  ping();
  // Use the fixed block size for buffers
  uint8_t passBuffer[MAX_PASS_LEN];
  char layout;
  char version;
  Preferences preferences;
  String result = String(DUMP_START) + "\n";

  for (int id = 0; id < MAX_PASSWORDS; id++) {
    const char *key = mkEntryName(id);

    preferences.begin(key, true);
    // Check if this password slot is used
    String nameStr = preferences.getString(F_NAME);
    if (nameStr.length() == 0) {
      preferences.end();
      break; // No more passwords stored
    }

    // Get metadata
    version = preferences.getInt(F_FORMAT, 0);
    layout = preferences.getInt(F_LAYOUT, -1);

    // Get password data (fixed-size encrypted block)
    preferences.getBytes(F_PASSWORD, passBuffer, MAX_PASS_LEN);

    String line = dumpSinglePassword(nameStr.c_str(), (const char *)passBuffer,
                                     MAX_PASS_LEN, layout, version, id);
    preferences.end();
    result += line + "\n";
  }
  result += DUMP_END;
  result += "\n";

  return result;
}

static int savePassCount = 0;

static void savePassData(const char *name, const uint8_t *passwordData,
                         char layout, unsigned char version, int slot) {

  Preferences preferences;
  const char *entryName = mkEntryName(slot);
  preferences.begin(entryName, false);
  preferences.putInt(F_FORMAT, (int)version);
  preferences.putInt(F_LAYOUT, (int)layout);
  preferences.putString(F_NAME, name);
  preferences.putBytes(F_PASSWORD, passwordData, MAX_PASS_LEN);
  preferences.end();
  savePassCount++;
}

String restoreMCUPasswords(const String &data) {
  savePassCount = 0;
  restorePasswords(data, savePassData);
  return String("Restored ") + String(savePassCount) +
         " passwords successfully.";
}
