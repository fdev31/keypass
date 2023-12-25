#ifndef _PASSWORD_H
#define _PASSWORD_H
#include <stdint.h>
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev
                               //
#include "hid.h"
//
// Define a struct to represent a password
typedef struct {
    char name[30];
    uint8_t layout;
    char password[40];
} Password;


extern AsyncWebServer server;
extern Password readPassword(int id);
extern void writePassword(int id, const Password* password);
extern void setUpKeyboard(AsyncWebServer &server);

#endif
