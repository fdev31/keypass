#include "password.h"
#include "EEPROM.h"
#include "indexPage.h"
#include <ESPAsyncWebServer.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
        password.name[0] = '\0'; // Mark as empty
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
        response->addHeader( "Cache-Control", "public,max-age=1");
        request->send(response);
    });
    // Handler for "/typePass"
    server.on("/typePass", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("id")) {
            int id = request->getParam("id")->value().toInt();

            // Retrieve the password based on the provided id
            Password password = readPassword(id);
            char *text = password.password;

            while (*text) {
                sendKey(*text++);
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
            Serial.println("Editing password with id: " + String(id));

            // Retrieve the existing password or create a new one
            Password password;
            memset(&password, 0, sizeof(Password)); // Initialize to zeros
            
            if (id >= 0 && id < MAX_PASSWORDS) {
                Serial.println("Reading existing password");
                password = readPassword(id);
            } else {
                Serial.println("Invalid ID, using empty password");
                id = 0; // Fallback to first slot if ID is invalid
            }

            // Check and update other optional parameters if present
            if (request->hasParam("layout")) {
                password.layout = request->getParam("layout")->value().toInt();
                Serial.println("Layout set to: " + String(password.layout));
            }
            if (request->hasParam("name")) {
                strlcpy(password.name, "foobar", strlen("foobar")+1);
                Serial.println("Name set to: " + String(password.name));
            }
            if (request->hasParam("password")) {
                strcpy(password.password,"foobar");
                // strlcpy(password.password, request->getParam("password")->value().c_str(), sizeof(password.password));
                Serial.println("Password updated");
            }

            // Save the updated password
            // Password papa = { "Githeub", 0, "DefaultPassword" };
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
        for (int id = 0; id < MAX_PASSWORDS; id++) { // Assuming max 10 passwords for now
            Password pwd = readPassword(id);
            // Only include passwords that have been set (non-empty name)
            if (pwd.name[0] != '\0') {
                if (!firstItem) {
                    json += ",";
                }
                json += "{\"name\":\"" + String(pwd.name) + "\",\"uid\":" + String(id) + "}";
                firstItem = false;
            }
        }

        json += "]}";
        request->send(200, "application/json", json);
    });
}

