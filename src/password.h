#ifndef _PASSWORD_H
#define _PASSWORD_H
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev
#include <stdint.h>
//
#include "hid.h"

#define MAX_PASSWORDS 100
#define MAX_NAME_LEN 30
#define MAX_PASS_LEN 40

typedef struct {
  char name[MAX_NAME_LEN];
  uint8_t layout;
  char password[MAX_PASS_LEN];
} Password;

extern AsyncWebServer server;
extern Password readPassword(int id);
extern void writePassword(int id, const Password *password);
extern void setUpKeyboard(AsyncWebServer &server);

#endif
