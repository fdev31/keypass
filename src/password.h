#include "constants.h"
#ifndef _PASSWORD_H
#define _PASSWORD_H
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev
#include <stdint.h>
//
#include "hid.h"

typedef struct {
  char name[MAX_NAME_LEN];
  int8_t layout;
  byte password[MAX_PASS_LEN];
  // NOTE: following are not defined since they are not used in property API
  // uint8_t pass_len;
} Password;

extern AsyncWebServer server;
extern void setUpKeyboard(AsyncWebServer &server);
extern bool typeRawText(const char *text, int layout, bool sendNewline);
extern bool typePassword(int id, int layout = -1, bool sendNewline = true);
extern const char *fetchPassword(int id);
extern bool editPassword(int id, const char *name = nullptr,
                         const char *password = nullptr, int layout = -2);
extern String listPasswords();
extern void factoryReset();
extern bool setWifiPassword(const char *pass);
extern bool setupPassphrase(const char *phrase);
extern String dumpPasswords();
extern int restorePasswords(const String &data);
#endif
