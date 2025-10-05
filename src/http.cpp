#include "settings.h"
#include "constants.h"
#include "crypto.h"
#include "graphics.h"
#include "hid.h"
#include "indexPage.h"
#include "main.h"
#include "password.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <StreamString.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ArduinoJson.h>

// HTTP handler wrappers (dependent on AsyncWebServerRequest)
//
static void handleRestore(AsyncWebServerRequest *request, uint8_t *data,
                          size_t len, size_t index, size_t total) {
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
      if (!request->_tempObject) {
        return request->send(400, "text/plain", "Nothing uploaded");
      }

      StreamString *buffer =
          reinterpret_cast<StreamString *>(request->_tempObject);
      String uploadData = buffer->readString();

      String ret = restoreMCUPasswords(uploadData);

      // Send response
      request->send(200, "text/plain", ret);
    }
  } else {
    request->send(500, "text/plain", "Failed to allocate buffer");
  }
}

static void handleRestoreComplete(AsyncWebServerRequest *request) {
  // This function is called when the request completes
  // If we get here without the handler sending a response, send one now
  if (!request->_tempObject) {
    request->send(400, "text/plain", "No data received");
  }
}
static void handleTypeRaw(AsyncWebServerRequest *request) {
  if (request->hasParam("text")) {
    const char *text = request->getParam("text")->value().c_str();
    int layout = 0; // Default layout
    if (request->hasParam("layout")) {
      layout = request->getParam("layout")->value().toInt();
    }

    bool sendNewline = !(request->hasParam("ret") &&
                         request->getParam("ret")->value() == "false");

    if (typeRawText(text, layout, sendNewline)) {
      request->send(200, "text/plain", "OK");
    } else {
      request->send(500, "text/plain", "Failed to type text");
    }
  } else {
    request->send(400, "text/plain", "Missing 'text' parameter");
  }
}

static void handleIndex(AsyncWebServerRequest *request) {
  ping();
  AsyncWebServerResponse *response =
      request->beginResponse(200, "text/html", index_html, index_html_gz_len);
  response->addHeader("Content-Encoding", "gzip");
  response->addHeader("Cache-Control", "public,max-age=1");
  request->send(response);
}

static void handleTypePass(AsyncWebServerRequest *request) {
  if (request->hasParam("id")) {
    int id = request->getParam("id")->value().toInt();
    int layout = -1;

    if (request->hasParam("layout")) {
      layout = request->getParam("layout")->value().toInt();
    }

    bool sendNewline = !(request->hasParam("ret") &&
                         request->getParam("ret")->value() == "false");

    if (typePassword(id, layout, sendNewline)) {
      request->send(200, "text/plain", "Password typed successfully");
    } else {
      request->send(400, "text/plain", "Invalid password ID");
    }
  } else {
    request->send(400, "text/plain", "Missing 'id' parameter");
  }
}

static void handleFetchPass(AsyncWebServerRequest *request) {
  if (request->hasParam("id")) {
    int id = request->getParam("id")->value().toInt();
    const char *password = fetchPassword(id);

    if (password) {
      request->send(200, "text/plain", password);
      clearFetchedPassword();
    } else {
      request->send(400, "text/plain", "Invalid password ID");
    }
  } else {
    request->send(400, "text/plain", "Missing 'id' parameter");
  }
}

static void handleEditPass(AsyncWebServerRequest *request) {
  if (request->hasParam("id")) {
    int id = request->getParam("id")->value().toInt();

    const char *name = nullptr;
    const char *password = nullptr;
    int layout = -2; // Special value meaning "don't change"

    if (request->hasParam("name")) {
      name = request->getParam("name")->value().c_str();
    }

    if (request->hasParam("password")) {
      password = request->getParam("password")->value().c_str();
    }

    if (request->hasParam("layout")) {
      layout = request->getParam("layout")->value().toInt();
    }

    if (editPassword(id, name, password, layout)) {
      request->send(200, "text/plain", "Password edited successfully");
    } else {
      request->send(400, "text/plain", "Invalid parameter");
    }
  } else {
    request->send(400, "text/plain", "Missing 'id' parameter");
  }
}

static void handleList(AsyncWebServerRequest *request) {
  String json = listPasswords();
  request->send(200, "application/json", json);
}

static void handleFactoryReset(AsyncWebServerRequest *request) {
  factoryReset();
  request->send(200, "text/plain", "Factory reset completed");
}

static void handleWifiPass(AsyncWebServerRequest *request) {
  if (request->hasParam("newPass")) {
    const char *pass = request->getParam("newPass")->value().c_str();
    if (setWifiPassword(pass)) {
      request->send(200, "text/plain", "WiFi password updated");
    } else {
      request->send(500, "text/plain", "Failed to update WiFi password");
    }
  } else {
    request->send(400, "text/plain", "Missing 'newPass' parameter");
  }
}

void handlePassphrase(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    StaticJsonDocument<100> doc;

    DeserializationError error = deserializeJson(doc, (const char*)data);

    if (error) {
        request->send(400, "text/plain", "Invalid JSON");
        return;
    }

    if (doc.containsKey("p")) {
        const char* phrase = doc["p"];
        if (setupPassphrase(phrase)) {
            request->send(200, "text/plain", "Passphrase set successfully");
        } else {
            request->send(500, "text/plain", "Passphrase couldn't be set");
        }
    } else {
        request->send(400, "text/plain", "Missing parameter");
    }
}

static void handlePassDump(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/plain");
  StringStreamAdapter stream;
  dumpPasswords(&stream);
  response->print(stream.toString());
  request->send(response);
}

void setupHttp(AsyncWebServer &server) {
#if USE_EEPROM_API
  EEPROM.begin(sizeof(Password) * MAX_PASSWORDS);
#endif

  server.on("/", HTTP_ANY, handleIndex);
  server.on("/typeRaw", HTTP_GET, handleTypeRaw);
  server.on("/typePass", HTTP_GET, handleTypePass);
  server.on("/editPass", HTTP_GET, handleEditPass);
  server.on("/fetchPass", HTTP_GET, handleFetchPass);
  server.on("/list", HTTP_GET, handleList);
  server.on("/passphrase", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, handlePassphrase);
  server.on("/reset", HTTP_GET, handleFactoryReset);
  server.on("/updateWifiPass", HTTP_GET, handleWifiPass);
  server.on("/dump", HTTP_GET, handlePassDump);
  // http post http://4.3.2.1/restore  < /tmp/dump
  //
  server.on("/restore", HTTP_POST, handleRestoreComplete,
            nullptr, // We're not using the normal handler here
            handleRestore);
}
