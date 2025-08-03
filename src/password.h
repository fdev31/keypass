#include "constants.h"
#ifndef _PASSWORD_H
#define _PASSWORD_H
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev

#include "hid.h"
#include "streamadapter.h"
#include <stdint.h>

typedef struct {
  char name[MAX_NAME_LEN];
  int8_t layout;
  char password[MAX_PASS_LEN];
} Password;

extern AsyncWebServer server;
extern bool dumpOnePassword(int id, StringStreamAdapter &pString);
extern void dumpPasswords(StringStreamAdapter *stream);
extern bool typeRawText(const char *text, int layout, bool sendNewline);
extern bool typePassword(int id, int layout = -1, bool sendNewline = true);
extern const char *fetchPassword(int id);
extern void clearFetchedPassword();
extern bool editPassword(int id, const char *name = nullptr,
                         const char *password = nullptr, int layout = -2);
extern String listPasswords();
extern void factoryReset();
extern bool setupPassphrase(const char *phrase);
extern String restoreMCUPasswords(const String &data);

extern void savePassData(const char *name, const char *passwordData, int layout,
                         int slot, uint8_t *nonce);
#endif
