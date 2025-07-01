#include "constants.h"
#ifndef _PASSWORD_H
#define _PASSWORD_H
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev
#include <stdint.h>
//
#include "hid.h"

typedef struct {
  char name[MAX_NAME_LEN];
  uint8_t layout;
  byte password[MAX_PASS_LEN];
  // NOTE: following are not defined since they are not used in property API
  // uint8_t pass_len;
} Password;

extern AsyncWebServer server;
extern void setUpKeyboard(AsyncWebServer &server);
#endif
