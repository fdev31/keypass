#include "password.h"
#include "EEPROM.h"
#include "indexPage.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Function to generate a random character
char getRandomChar() {
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0"
                         "123456789!@#$%^&*()_+-=[]{}|;:,.<>?";
  const size_t charsetSize = sizeof(charset) - 1;
  return charset[rand() % charsetSize];
}

// Function to generate a random password of given length
void generatePassword(char *password, int length) {
  for (int i = 0; i < length; ++i) {
    password[i] = getRandomChar();
  }
  password[length] = '\0'; // Null-terminate the string
}

// Function to read a password from EEPROM
Password readPassword(int id) {
  Password password;
  int address = id * sizeof(Password);
  EEPROM.get(address, password);
  // Add validation check to ensure data is valid
  if (password.name[0] == 0xFF) { // EEPROM default state is 0xFF
    password.name[0] = '\0';      // Mark as empty
  }
  return password;
}

// Function to write a password to EEPROM
void writePassword(int id, const Password &password) {
  int address = id * sizeof(Password);
  EEPROM.put(address, password);
  EEPROM.commit();
}

void setUpKeyboard(AsyncWebServer &server) {

  EEPROM.begin(sizeof(Password) * MAX_PASSWORDS);
  // Write a basic password as the first one to ensure valid data
  // Password password = { "Default", 0, "DefaultPassword" };
  // EEPROM.put(0, password);

  // Serve Basic HTML Page
  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response =
        request->beginResponse(200, "text/html", index_html);
    response->addHeader("Cache-Control", "public,max-age=1");
    request->send(response);
  });
  // Handler for "/typePass"
  server.on("/typePass", HTTP_GET, [](AsyncWebServerRequest *request) {
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
  });

  server.on("/editPass", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("id")) {
      int id = request->getParam("id")->value().toInt();

      // Retrieve the existing password or create a new one
      Password password;
      memset(&password, 0, sizeof(Password)); // Initialize to zeros

      if (id >= 0 && id < MAX_PASSWORDS) {
        password = readPassword(id);
      } else {
        id = 0; // Fallback to first slot if ID is invalid
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
  });
  // Handler for "/list"
  server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Create a dynamic JSON string to hold the list of passwords
    String json = "{\"passwords\":[";

    // Loop through password ids and add existing passwords to the JSON
    bool firstItem = true;
    for (int id = 0; id < MAX_PASSWORDS;
         id++) { // Assuming max 10 passwords for now
      Password pwd = readPassword(id);
      // Only include passwords that have been set (non-empty name)
      if (pwd.name[0] != '\0') {
        if (!firstItem) {
          json += ",";
        }
        json += "{\"name\":\"" + String(pwd.name) + "\",\"uid\":" + String(id) +
                ",\"layout\":" + String(pwd.layout) + "}";
        firstItem = false;
      }
    }

    json += "]}";
    request->send(200, "application/json", json);
  });
}
