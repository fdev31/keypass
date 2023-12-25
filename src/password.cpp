#include "EEPROM.h"
#include "password.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "indexPage.h"

// Function to generate a random character
char getRandomChar() {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{}|;:,.<>?";
    const size_t charsetSize = sizeof(charset) - 1;
    return charset[rand() % charsetSize];
}

// Function to generate a random password of given length
void generatePassword(char *password, int length) {
    for (int i = 0; i < length; ++i) {
        password[i] = getRandomChar();
    }
    password[length] = '\0';  // Null-terminate the string
}

int main() {
    srand((unsigned int)time(NULL));  // Seed the random number generator

    int passwordLength;
    printf("Enter the desired password length: ");
    scanf("%d", &passwordLength);

    if (passwordLength <= 0) {
        printf("Invalid password length.\n");
        return 1;
    }

    char password[passwordLength + 1];  // +1 for null-termination

    generatePassword(password, passwordLength);

    printf("Generated Password: %s\n", password);

    return 0;
}


// Function to read a password from EEPROM
Password readPassword(int id) {
    Password password;
    int address = id * sizeof(Password);
    EEPROM.get(address, password);
    return password;
}

// Function to write a password to EEPROM
void writePassword(int id, const Password* password) {
    int address = id * sizeof(Password);
    EEPROM.put(address, password);
    EEPROM.commit();
}

void setUpKeyboard(AsyncWebServer &server) {
    // Serve Basic HTML Page
    server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
            AsyncWebServerResponse *response =
            request->beginResponse(200, "text/html", index_html);
            // Add "<script>foo()</script>" to the response
            response->addHeader(
                    "Cache-Control",
                    "public,max-age=1"); // save this file to cache for 1 year
                                         //        "public,max-age=31536000"); // save this file to cache for 1 year
                                         // (unless you refresh)
            request->send(response);
    });
    server.on("/action", HTTP_ANY, [](AsyncWebServerRequest *request) {
            const char *text = "This is a message";
            while (*text) {
            sendKey(*text++);
            }

            AsyncWebServerResponse *response =
            request->beginResponse(200, "text/html", index_html);
            response->addHeader("Cache-Control",
                    "public,max-age=1"); // save this file to cache for 1
                                         // year (unless you refresh)
            request->send(response);
            });
    // Handler for "/typePass"
    server.on("/typePass", HTTP_GET, [](AsyncWebServerRequest *request) {
            if (request->hasParam("id")) {
            int id = request->getParam("id")->value().toInt();

            // Retrieve the password based on the provided id
            Password password = readPassword(id);

            // Check if the "validate" parameter is present
            bool validate = request->hasParam("validate") && request->getParam("validate")->value().equalsIgnoreCase("true");

            // Perform the password typing logic here using the retrieved password and the "validate" parameter
            // ...

            // Send response if needed
            request->send(200, "text/plain", "Password typed successfully");
            } else {
            request->send(400, "text/plain", "Missing 'id' parameter");
            }
            });

    // Handler for "/editPass"
    server.on("/editPass", HTTP_GET, [](AsyncWebServerRequest *request) {
            if (request->hasParam("id")) {
            int id = request->getParam("id")->value().toInt();

            // Retrieve the existing password or create a new one
            Password password;
            if (id >= 0) {
            password = readPassword(id);
            }

            // Check and update other optional parameters if present
            if (request->hasParam("layout")) {
            password.layout = request->getParam("layout")->value().toInt();
            }
            if (request->hasParam("password")) {
            strlcpy(password.password, request->getParam("password")->value().c_str(), sizeof(password.password));
            }

            // Save the updated password
            writePassword(id, &password);

            // Send response if needed
            request->send(200, "text/plain", "Password edited successfully");
            } else {
                request->send(400, "text/plain", "Missing 'id' parameter");
            }
    });

    // Handler for "/list"
    server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request) {
            // Construct and send the JSON list of passwords
            // ...

            request->send(200, "application/json", "{\"passwords\":[{\"name\":\"password1\",\"layout\":1,\"password\":\"pass123\"},{\"name\":\"password2\",\"layout\":2,\"password\":\"pass456\"}]}");
            });
}
