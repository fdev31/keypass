#ifndef __CRYPTO_H
#define __CRYPTO_H
#include "password.h"
#include "xxhash.h"
#include <Crypto.h>
#include <Speck.h>
#include <SpeckSmall.h>
#include <SpeckTiny.h>
#include <string.h>

extern void encryptPassword(const char *password, uint8_t *result);
extern void decryptPassword(const uint8_t *password, char *result);
// extern const byte *encryptPassword(const uint8_t *password);
extern bool setPassPhrase(const char *passphrase, unsigned long pin);
#endif
