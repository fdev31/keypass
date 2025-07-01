#include "configuration.h"
#include "hid.h"

#include "Preferences.h"
#include "crypto.h"
#include "indexPage.h"
#include "password.h"
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
extern int sleeping;

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
  size_t pass_version = preferences.getInt("v");
  strlcpy(password.name, preferences.getString("name").c_str(), MAX_NAME_LEN);
  preferences.getBytes("password", buffer, MAX_PASS_LEN);
  password.layout = preferences.getInt("layout", -1);
  preferences.end();

  if (pass_version == 0) { // unencrypted version 0
    strlcpy((char *)password.password, (char *)buffer, MAX_PASS_LEN);
  } else { // current version using XXH32 + SPECK
    decryptPassword((uint8_t *)buffer, (char *)password.password);
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
  static byte encrypted_password[MAX_PASS_LEN];
  // initialize encrypted_password with random values
  encryptPassword((char *)password.password, encrypted_password);
  preferences.putBytes("password", encrypted_password, MAX_PASS_LEN);
  preferences.putInt("v", FORMAT_VERSION);
  preferences.putInt("layout", password.layout);
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
static void handleTypeRaw(AsyncWebServerRequest *request) {
  if (request->hasParam("text")) {
    ping();
    const char *text = request->getParam("text")->value().c_str();
    int layout = 0; // Default layout
    if (request->hasParam("layout")) {
      layout = request->getParam("layout")->value().toInt();
    }

    sendAnyKeymap(text, layout,
                  !(request->hasParam("ret") &&
                    request->getParam("ret")->value() == "false"));
    request->send(200, "text/plain", "OK");
  } else {
    request->send(400, "text/plain", "Missing 'text' parameter");
  }
}

static void handleIndex(AsyncWebServerRequest *request) {
  ping();
#ifdef ENABLE_GRAPHICS
  strlcpy(DEBUG_BUFFER, "Welcome !", 99);
#endif
  AsyncWebServerResponse *response =
      request->beginResponse(200, "text/html", index_html);
  response->addHeader("Cache-Control", "public,max-age=1");
  request->send(response);
}

static void handleTypePass(AsyncWebServerRequest *request) {
  Password password;
  ping();
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

    sendAnyKeymap(text, layout,
                  !(request->hasParam("ret") &&
                    request->getParam("ret")->value() == "false"));
    // Send response if needed
    request->send(200, "text/plain", "Password typed successfully");
  } else {
    request->send(400, "text/plain", "Missing 'id' parameter");
  }
}

static void handleFetchPass(AsyncWebServerRequest *request) {
  Password password;
  ping();
  if (request->hasParam("id")) {
    int id = request->getParam("id")->value().toInt();
    password = readPassword(id);
    return request->send(200, "text/plain", (char *)password.password);
  }
}
static void handleEditPass(AsyncWebServerRequest *request) {
  Password password;
  ping();
#ifdef ENABLE_GRAPHICS
  // strlcpy(DEBUG_BUFFER, "Edited", 99);
#endif
  if (request->hasParam("id")) {
    int id = request->getParam("id")->value().toInt();

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
  request->send(200, "application/json", json);
}

static void handleFactoryReset(AsyncWebServerRequest *request) {
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
    ping();
    preferences.begin("KeyPass", false);
    preferences.putString("wifi_password", pass);
    preferences.end();
  }
}

static void handlePassPhrase(AsyncWebServerRequest *request) {
  if (request->hasParam("p") && request->hasParam("k")) {
    ping();
    const char *phrase = request->getParam("p")->value().c_str();
    unsigned long pin = request->getParam("k")->value().toInt();
    if (setPassPhrase(phrase, pin)) {
      request->send(200, "text/plain", "Passphrase set successfully");
    } else {
      request->send(500, "text/plain", "Passphrase couldn't be set");
    }
  } else {
    request->send(400, "text/plain", "Missing 'p' or 'k' parameter");
  }
}

String hexDump(const uint8_t *data, size_t len) {
  String result = "";
  char hexChars[3]; // Two characters for the hex value plus null terminator
  //

  for (size_t i = 0; i < len; i++) {
    // Convert each byte to a two-character hex representation
    sprintf(hexChars, "%02X", data[i]);
    result += hexChars;
  }

  return result;
}

static void handlePassDump(AsyncWebServerRequest *request) {
  ping();
  char buffer[MAX_PASS_LEN];
  Preferences preferences;
  AsyncResponseStream *response = request->beginResponseStream("text/plain");

  response->print("#KPDUMP\n");

  for (int id = 0; id < MAX_PASSWORDS; id++) {
    const char *key = mkEntryName(id);
    *buffer = 0;
    preferences.begin(key, true);
    preferences.getBytes("password", buffer, MAX_PASS_LEN);
    preferences.end();
    if (!*buffer)
      break;

    response->print(hexDump((uint8_t *)buffer, MAX_PASS_LEN));
    response->print("\n");
  }
  response->print("#/KPDUMP\n");

  request->send(response);
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

void handleRestore(AsyncWebServerRequest *request) {
  if (!request->_tempObject) {
    return request->send(400, "text/plain", "Nothing uploaded");
  }
  ping();

  StreamString *buffer = reinterpret_cast<StreamString *>(request->_tempObject);
  int bodyLength = request->contentLength();
  int pos = 0;
  int slot = 0;

  char hexBuffer[MAX_PASS_LEN * 2 + 1];
  uint8_t binData[MAX_PASS_LEN];
  Preferences preferences;
  bool insideKpDump = false;

  // Process each line from the request body
  while (slot < MAX_PASSWORDS && pos < bodyLength) {
    int lineEnd = buffer->indexOf('\n', pos);
    if (lineEnd == -1) {
      lineEnd = bodyLength;
    }

    // Skip empty lines
    if (lineEnd - pos <= 0) {
      pos = lineEnd + 1; // Move past the newline
      continue;
    }

    // Copy line to hexBuffer with bounds checking
    int lineLen = min(lineEnd - pos, (int)sizeof(hexBuffer) - 1);
    String currentLine = buffer->substring(pos, pos + lineLen);
    currentLine.toCharArray(hexBuffer, lineLen + 1);

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
    if (insideKpDump) {
      // Convert hex string back to binary data
      hexParse(hexBuffer, binData, MAX_PASS_LEN);

      // Save the binary data directly to preferences
      preferences.begin(mkEntryName(slot), false);
      preferences.putBytes("password", binData, MAX_PASS_LEN);
      preferences.end();

      slot++;
    }

    pos = lineEnd + 1; // Move past the newline
  }

  // Send response
  String response = "{\"status\":\"success\",\"message\":\"Restored " +
                    String(slot) + " passwords\"}";
  request->send(200, "application/json", response);
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
  server.on("/dump", HTTP_GET, handlePassDump);
  // http post http://4.3.2.1/restore  < /tmp/dump
  server.on(
      "/restore", HTTP_POST,
      [](AsyncWebServerRequest *request) {
        // This function is called when the request completes
        // If we get here without the handler sending a response, send one now
        if (!request->_tempObject) {
          request->send(400, "text/plain", "No data received");
        }
      },
      nullptr, // We're not using the normal handler here
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
         size_t index, size_t total) {
        // This is the body handler for non-multipart uploads
        if (index == 0) {
          // First chunk - initialize the buffer
          request->_tempObject = new StreamString();
        }

        if (request->_tempObject) {
          StreamString *buffer =
              reinterpret_cast<StreamString *>(request->_tempObject);
          // Add this chunk to the buffer
          buffer->write(data, len);

          if (index + len == total) {
            // This is the last chunk, process the complete data
            handleRestore(request);
          }
        } else {
          request->send(500, "text/plain", "Failed to allocate buffer");
        }
      });

#ifdef ENABLE_GRAPHICS
  Preferences prefs;
  size_t entries_left = prefs.freeEntries();
  strlcpy(DEBUG_BUFFER2, (String(entries_left) + String(" left.")).c_str(), 99);
#endif
}
