
#include <Arduino.h>
#include <cstdint>

typedef void (*PasswordCallback)(const char *name, const char *passwordData,
                                 int layout, int slot, uint8_t *nonce);

void restoreSinglePassword(const String &passBlock, PasswordCallback callback,
                           int slot);
extern int restorePasswords(const String &data, PasswordCallback callback,
                            bool);
