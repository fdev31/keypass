#include "password.h"
#include "Preferences.h"
#include "configuration.h"
#include "constants.h"
#include "crypto.h"
#include "hid.h"
#include "indexPage.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <StreamString.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

extern void ping();

#ifdef ENABLE_GRAPHICS
extern char DEBUG_BUFFER[100];
extern char DEBUG_BUFFER2[100];
#endif

// Core business logic functions (independent of AsyncWebServerRequest)

const char *mkEntryName(int num) {
  static char name[16]; // Static buffer to hold the result
  snprintf(name, sizeof(name), "p%d", num);
  return name;
}

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
  static Password password;
  static uint8_t buffer[MAX_PASS_LEN];
  Preferences preferences;
  const char *key = mkEntryName(id);
  preferences.begin(key, true);
  size_t pass_version = preferences.getInt(F_FORMAT, 0);
  strlcpy(password.name, preferences.getString(F_NAME).c_str(), MAX_NAME_LEN);
  preferences.getBytes(F_PASSWORD, buffer, MAX_PASS_LEN);
  password.layout = preferences.getInt(F_LAYOUT, -1);
  preferences.end();

  if (pass_version == 0) { // unencrypted version 0
    strlcpy((char *)password.password, (char *)buffer, MAX_PASS_LEN);
  } else { // current version using XXH32 + SPECK
    decryptPassword((uint8_t *)buffer, (char *)password.password);
  }
  return password;
#endif
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
  // initialize encrypted_password with random values
  encryptPassword((char *)password.password, encrypted_password);
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

// Service functions (independent of AsyncWebServerRequest)

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
  strlcpy(DEBUG_BUFFER, "Shazzaam", 99);
#endif

  Password password = readPassword(id);
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
  Password password = readPassword(id);
  return (char *)password.password;
}

bool editPassword(int id, const char *name, const char *password, int layout) {
  if (id < 0 || id >= MAX_PASSWORDS)
    return false;

  ping();
#ifdef ENABLE_GRAPHICS
  // strlcpy(DEBUG_BUFFER, "Edited", 99);
#endif

  Password pwd = readPassword(id);

  // Update fields if provided
  if (layout != -2) { // -2 means "don't change"
    pwd.layout = layout;
  }

  if (name != nullptr) {
    strlcpy(pwd.name, name, MAX_NAME_LEN);
  }

  if (password != nullptr) {
    strlcpy((char *)pwd.password, password, MAX_PASS_LEN);
  }

  // Save the updated password
  writePassword(id, pwd);
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
  strlcpy(DEBUG_BUFFER, "Reset", 99);
#endif
  Preferences preferences;

  for (int id = 0; id < MAX_PASSWORDS; id++) {
    Password emptyPassword;
    memset(&emptyPassword, 0, sizeof(Password)); // Initialize to zeros
    writePassword(id, emptyPassword);
  }

#if not USE_EEPROM_API
  preferences.begin(F_NAMESPACE, false);
  preferences.clear(); // Clear all preferences
  preferences.end();
#elif
// TODO: implement this
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

bool setupPassphrase(const char *phrase, unsigned long pin) {
  if (phrase == nullptr)
    return false;
  ping();
  return setPassPhrase(phrase, pin);
}

String hexDump(const uint8_t *data, size_t len) {
  String result = "";
  char hexChars[3]; // Two characters for the hex value plus null terminator

  for (size_t i = 0; i < len; i++) {
    // Convert each byte to a two-character hex representation
    sprintf(hexChars, "%02X", data[i]);
    result += hexChars;
  }

  return result;
}

String dumpPasswords() {
  ping();
  char buffer[MAX_PASS_LEN];
  Preferences preferences;
  String result = "#KPDUMP\n";

  for (int id = 0; id < MAX_PASSWORDS; id++) {
    const char *key = mkEntryName(id);
    *buffer = 0;
    preferences.begin(key, true);
    preferences.getBytes("password", buffer, MAX_PASS_LEN);
    preferences.end();
    if (!*buffer)
      break;

    result += hexDump((uint8_t *)buffer, MAX_PASS_LEN);
    result += "\n";
  }
  result += "#/KPDUMP\n";

  return result;
}

// Parse a hex string back into binary data
void hexParse(const char *hexStr, uint8_t *binData, size_t maxLen) {
  size_t len = 0;
  while (*hexStr && *(hexStr + 1) && len < maxLen) {
    char highNibble = *hexStr++;
    char lowNibble = *hexStr++;

    uint8_t value = 0;
    if (highNibble >= '0' && highNibble <= '9')
      value = (highNibble - '0') << 4;
    else if (highNibble >= 'A' && highNibble <= 'F')
      value = (highNibble - 'A' + 10) << 4;
    else if (highNibble >= 'a' && highNibble <= 'f')
      value = (highNibble - 'a' + 10) << 4;

    if (lowNibble >= '0' && lowNibble <= '9')
      value |= (lowNibble - '0');
    else if (lowNibble >= 'A' && lowNibble <= 'F')
      value |= (lowNibble - 'A' + 10);
    else if (lowNibble >= 'a' && lowNibble <= 'f')
      value |= (lowNibble - 'a' + 10);

    binData[len++] = value;
  }
}

int restorePasswords(const String &data) {
  ping();
  int slot = 0;
  uint8_t binData[MAX_PASS_LEN];
  Preferences preferences;
  bool insideKpDump = false;

  // Process each line from the data
  int pos = 0;
  while (slot < MAX_PASSWORDS && pos < data.length()) {
    int lineEnd = data.indexOf('\n', pos);
    if (lineEnd == -1) {
      lineEnd = data.length();
    }

    // Skip empty lines
    if (lineEnd - pos <= 0) {
      pos = lineEnd + 1; // Move past the newline
      continue;
    }

    String currentLine = data.substring(pos, lineEnd);

    // Check for KPDUMP markers
    if (currentLine == "#KPDUMP") {
      insideKpDump = true;
      pos = lineEnd + 1; // Move past the marker
      continue;
    } else if (currentLine == "#/KPDUMP") {
      insideKpDump = false;
      break; // End of dump data
    }

    // Only process lines if we're inside a KPDUMP section or if no markers were
    // used
    if (insideKpDump || !data.startsWith("#KPDUMP")) {
      // Convert hex string back to binary data
      hexParse(currentLine.c_str(), binData, MAX_PASS_LEN);

      // Save the binary data directly to preferences
      preferences.begin(mkEntryName(slot), false);
      preferences.putBytes("password", binData, MAX_PASS_LEN);
      preferences.end();

      slot++;
    }

    pos = lineEnd + 1; // Move past the newline
  }

  return slot;
}
