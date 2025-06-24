#include "configuration.h"

#include "Preferences.h"
#include "crypto.h"
#include "indexPage.h"
#include "password.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

extern unsigned long lastClientTime;

#ifdef ENABLE_GRAPHICS
extern size_t roundPassLength(size_t size);
extern char DEBUG_BUFFER[100];
extern char DEBUG_BUFFER2[100];
#endif
extern int sleeping;

static void handleTypeRaw(AsyncWebServerRequest *request);
static void handleTypePass(AsyncWebServerRequest *request);
static void handleIndex(AsyncWebServerRequest *request);
static void handleEditPass(AsyncWebServerRequest *request);
static void handleFetchPass(AsyncWebServerRequest *request);
static void handleList(AsyncWebServerRequest *request);
static void handleFactoryReset(AsyncWebServerRequest *request);
static void handleWifiPass(AsyncWebServerRequest *request);
static void handlePassPhrase(AsyncWebServerRequest *request);

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
  static byte buffer[MAX_PASS_LEN];
  Preferences preferences;
  const char *key = mkEntryName(id);
  preferences.begin(key, true);
  size_t pass_len = preferences.getInt("pass_len");
  strlcpy(password.name, preferences.getString("name").c_str(), MAX_NAME_LEN);
  preferences.getBytes("password", buffer, MAX_PASS_LEN);
  password.layout = preferences.getInt("layout", -1);
  preferences.end();

  if (pass_len) { // encrypted
    decryptPassword((uint8_t *)buffer, (char *)password.password);
  } else {
    strlcpy((char *)password.password, (char *)buffer, MAX_PASS_LEN);
  }

  return password;
#endif
}

// Function to write a password to EEPROM
static void writePassword(int id, const Password &password) {
#if USE_EEPROM_API
  int address = id * sizeof(Password);
  EEPROM.put(address, password);
  EEPROM.commit();
#else
  Preferences preferences;
  preferences.begin(mkEntryName(id), false);
  preferences.putString("name", password.name);
  int pass_len = strlen((char *)password.password);
  byte encrypted_password[MAX_PASS_LEN] = {0};
  encryptPassword((char *)password.password, encrypted_password);
  preferences.putBytes("password", encrypted_password, MAX_PASS_LEN);
  preferences.putInt("pass_len", pass_len);
  preferences.putInt("layout", password.layout);
  preferences.end();
#endif
}

static void handleTypeRaw(AsyncWebServerRequest *request) {
  if (request->hasParam("text")) {
    sleeping = 0;              // Reset sleeping state
    lastClientTime = millis(); // Reset the timer on each request
    const char *text = request->getParam("text")->value().c_str();
    int layout = 0; // Default layout
    if (request->hasParam("layout")) {
      layout = request->getParam("layout")->value().toInt();
    }

    while (*text) {
      sendKeymap(*text++, layout);
    }
    sendKey('\n');
    request->send(200, "text/plain", "OK");
  } else {
    request->send(400, "text/plain", "Missing 'text' parameter");
  }
}

static void handleIndex(AsyncWebServerRequest *request) {
  sleeping = 0; // Reset sleeping state
#ifdef ENABLE_GRAPHICS
  strlcpy(DEBUG_BUFFER, "Welcome !", 99);
#endif
  lastClientTime = millis(); // Reset the timer on each request
  AsyncWebServerResponse *response =
      request->beginResponse(200, "text/html", index_html);
  response->addHeader("Cache-Control", "public,max-age=1");
  request->send(response);
}

static void handleTypePass(AsyncWebServerRequest *request) {
  Password password;
  sleeping = 0;              // Reset sleeping state
  lastClientTime = millis(); // Reset the timer on each request
#ifdef ENABLE_GRAPHICS
  strlcpy(DEBUG_BUFFER, "Shazzaam", 99);
#endif
  if (request->hasParam("id")) {
    int id = request->getParam("id")->value().toInt();

    // Retrieve the password based on the provided id
    password = readPassword(id);
    char *text = (char *)password.password;

    if (request->hasParam("layout")) {
      password.layout = request->getParam("layout")->value().toInt();
    }

    int layout = password.layout;

    while (*text) {
      sendKeymap(*text++, layout);
    }
    sendKey('\n');

    // Send response if needed
    request->send(200, "text/plain", "Password typed successfully");
  } else {
    request->send(400, "text/plain", "Missing 'id' parameter");
  }
}

static void handleFetchPass(AsyncWebServerRequest *request) {
  Password password;
  sleeping = 0;              // Reset sleeping state
  lastClientTime = millis(); // Reset the timer on each request
  if (request->hasParam("id")) {
    int id = request->getParam("id")->value().toInt();
    password = readPassword(id);
    return request->send(200, "text/plain", (char *)password.password);
  }
}
static void handleEditPass(AsyncWebServerRequest *request) {
  Password password;
  sleeping = 0;              // Reset sleeping state
  lastClientTime = millis(); // Reset the timer on each request
#ifdef ENABLE_GRAPHICS
  // strlcpy(DEBUG_BUFFER, "Edited", 99);
#endif
  if (request->hasParam("id")) {
    int id = request->getParam("id")->value().toInt();

    // Retrieve the existing password or create a new one
    memset(&password, 0, sizeof(Password)); // Initialize to zeros

    if (id >= 0 && id < MAX_PASSWORDS) {
      password = readPassword(id);
    } else {
      request->send(400, "text/plain", "Invalid 'id' parameter");
      return;
    }

    // Check and update other optional parameters if present
    if (request->hasParam("layout")) {
      password.layout = request->getParam("layout")->value().toInt();
    }
    if (request->hasParam("name")) {
      const char *source = request->getParam("name")->value().c_str();
      strlcpy(password.name, source, MAX_NAME_LEN);
    }
    if (request->hasParam("password")) {
      const char *tmp = request->getParam("password")->value().c_str();
      strlcpy((char *)password.password, tmp, MAX_PASS_LEN);
    }

    // Save the updated password
    writePassword(id, password);

    // Send response if needed
    request->send(200, "text/plain", "Password edited successfully");
  } else {
    request->send(400, "text/plain", "Missing 'id' parameter");
  }
}

static void handleList(AsyncWebServerRequest *request) {
  Password pwd;
  sleeping = 0;              // Reset sleeping state
  lastClientTime = millis(); // Reset the timer on each request
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
  request->send(200, "application/json", json);
}

static void handleFactoryReset(AsyncWebServerRequest *request) {
  sleeping = 0;              // Reset sleeping state
  lastClientTime = millis(); // Reset the timer on each request
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
  preferences.begin("KeyPass", false);
  preferences.clear(); // Clear all preferences
  preferences.end();
#elif
// TODO: implement this
#pragma message "EEPROM API is used, factory reset not implemented for settings"
#endif
}

static void handleWifiPass(AsyncWebServerRequest *request) {
  if (request->hasParam("newPass")) {
    const char *pass = request->getParam("newPass")->value().c_str();
    Preferences preferences;
    sleeping = 0;              // Reset sleeping state
    lastClientTime = millis(); // Reset the timer on each request
    preferences.begin("KeyPass", false);
    preferences.putString("wifi_password", pass);
    preferences.end();
  }
}

static void handlePassPhrase(AsyncWebServerRequest *request) {
  sleeping = 0;              // Reset sleeping state
  lastClientTime = millis(); // Reset the timer on each request
  if (request->hasParam("p")) {
    const char *phrase = request->getParam("p")->value().c_str();
    setPassPhrase(phrase);
    request->send(200, "text/plain", "Passphrase set successfully");
  } else {
    request->send(400, "text/plain", "Missing 'phrase' parameter");
  }
}

void setUpKeyboard(AsyncWebServer &server) {

#if USE_EEPROM_API
  EEPROM.begin(sizeof(Password) * MAX_PASSWORDS);
#endif

  server.on("/", HTTP_ANY, handleIndex);
  server.on("/typeRaw", HTTP_GET, handleTypeRaw);
  server.on("/typePass", HTTP_GET, handleTypePass);
  server.on("/editPass", HTTP_GET, handleEditPass);
  server.on("/fetchPass", HTTP_GET, handleFetchPass);
  server.on("/list", HTTP_GET, handleList);
  server.on("/passphrase", HTTP_GET, handlePassPhrase);
  server.on("/reset", HTTP_GET, handleFactoryReset);
  server.on("/updateWifiPass", HTTP_GET, handleWifiPass);

#ifdef ENABLE_GRAPHICS
  Preferences prefs;
  size_t entries_left = prefs.freeEntries();
  strlcpy(DEBUG_BUFFER2, (String(entries_left) + String(" left.")).c_str(), 99);
#endif
}
