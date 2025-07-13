
#include <Arduino.h>
#include <cstdint>

typedef void (*PasswordCallback)(const char *name, const uint8_t *passwordData,
                                 char layout, unsigned char version, int slot);
extern int restorePasswords(const String &data, PasswordCallback callback,
                            bool);
