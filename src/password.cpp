#include "password.h"
#include "Preferences.h"
#include "indexPage.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

extern unsigned long lastClientTime;

#ifdef ENABLE_GRAPHICS
extern char DEBUG_BUFFER[100];
extern char DEBUG_BUFFER2[100];
#endif

void handleTypeRaw(AsyncWebServerRequest *request);
void handleTypePass(AsyncWebServerRequest *request);
void handleIndex(AsyncWebServerRequest *request);
void handleEditPass(AsyncWebServerRequest *request);
void handleList(AsyncWebServerRequest *request);

const char *mkEntryName(int num) {
  static char buffer[16]; // Static buffer to hold the result
  snprintf(buffer, sizeof(buffer), "p%d", num);
  return buffer;
}

Password readPassword(int id) {
#if USE_EEPROM_EMULATION
  Password password;
  int address = id * sizeof(Password);
  EEPROM.get(address, password);
  // Add validation check to ensure data is valid
  if (password.name[0] == 0xFF) { // EEPROM default state is 0xFF
    password.name[0] = '\0';      // Mark as empty
  }
  return password;
#else
  Preferences preferences;
  Password password;
  const char *key = mkEntryName(id);
  if (true) {
    preferences.begin(key, true);
    strlcpy(password.name, preferences.getString("name").c_str(), MAX_NAME_LEN);
    strlcpy(password.password, preferences.getString("password").c_str(),
            MAX_PASS_LEN);
    password.layout = preferences.getInt("layout", -1);

    preferences.end();
  } else {
    password.layout = -1;        // Mark as not initialized
    password.name[0] = '\0';     // Mark as empty
    password.password[0] = '\0'; // Mark as empty
  }
  return password;
#endif
}

// Function to write a password to EEPROM
void writePassword(int id, const Password &password) {
#if USE_EEPROM_EMULATION
  int address = id * sizeof(Password);
  EEPROM.put(address, password);
  EEPROM.commit();
#else
  Preferences preferences;
  preferences.begin(mkEntryName(id), false);
  preferences.putString("name", password.name);
  preferences.putString("password", password.password);
  preferences.putInt("layout", password.layout);
  preferences.end();
#endif
}

void handleTypeRaw(AsyncWebServerRequest *request) {
  if (request->hasParam("text")) {
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

void handleIndex(AsyncWebServerRequest *request) {
#ifdef ENABLE_GRAPHICS
  strlcpy(DEBUG_BUFFER, "Welcome !", 99);
#endif
  lastClientTime = millis(); // Reset the timer on each request
  AsyncWebServerResponse *response =
      request->beginResponse(200, "text/html", index_html);
  response->addHeader("Cache-Control", "public,max-age=1");
  request->send(response);
}

void handleTypePass(AsyncWebServerRequest *request) {
#ifdef ENABLE_GRAPHICS
  strlcpy(DEBUG_BUFFER, "Shazzaam", 99);
#endif
  millis(); // Reset the timer on each request
  if (request->hasParam("id")) {
    int id = request->getParam("id")->value().toInt();

    // Retrieve the password based on the provided id
    Password password = readPassword(id);
    char *text = password.password;
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

void handleEditPass(AsyncWebServerRequest *request) {
#ifdef ENABLE_GRAPHICS
  strlcpy(DEBUG_BUFFER, "Edited", 99);
#endif
  lastClientTime = millis(); // Reset the timer on each request
  if (request->hasParam("id")) {
    int id = request->getParam("id")->value().toInt();

    // Retrieve the existing password or create a new one
    Password password;
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
      strlcpy(password.password, tmp, MAX_PASS_LEN);
    }

    // Save the updated password
    writePassword(id, password);

    // Send response if needed
    request->send(200, "text/plain", "Password edited successfully");
  } else {
    request->send(400, "text/plain", "Missing 'id' parameter");
  }
}

void handleList(AsyncWebServerRequest *request) {
  lastClientTime = millis(); // Reset the timer on each request
  // Create a dynamic JSON string to hold the list of passwords
  String json = "{\"passwords\":[";

  // Loop through password ids and add existing passwords to the JSON
  bool firstItem = true;
  for (int id = 0; id < MAX_PASSWORDS; id++) {
    Password pwd = readPassword(id);
    if (pwd.name[0] == '\0')
      break;

    if (!firstItem) {
      json += ",";
    }
    json += "{\"name\":\"" + String(pwd.name) + "\",\"uid\":" + String(id) +
            ",\"layout\":" + String(pwd.layout) + "}";
    firstItem = false;
  }

  Preferences prefs;
  size_t entries_left = prefs.freeEntries();
  json += String("], \"free\":") + entries_left + "}";
  request->send(200, "application/json", json);
}

void setUpKeyboard(AsyncWebServer &server) {

#if USE_EEPROM_EMULATION
  EEPROM.begin(sizeof(Password) * MAX_PASSWORDS);
#endif

  server.on("/", HTTP_ANY, handleIndex);
  server.on("/typeRaw", HTTP_GET, handleTypeRaw);
  server.on("/typePass", HTTP_GET, handleTypePass);
  server.on("/editPass", HTTP_GET, handleEditPass);
  server.on("/list", HTTP_GET, handleEditPass);

#ifdef ENABLE_GRAPHICS
  Preferences prefs;
  size_t entries_left = prefs.freeEntries();
  strlcpy(DEBUG_BUFFER2, (String(entries_left) + String(" left.")).c_str(), 99);
#endif
}
