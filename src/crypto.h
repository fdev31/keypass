#include "constants.h"
#include <stdint.h>
#ifndef _CRYPTO_H
#define _CRYPTO_H

extern uint8_t *getNonce();
extern void encryptBuffer(const char *, uint8_t *result, int size,
                          uint8_t *nonce);
extern void decryptBuffer(const uint8_t *, char *result, int size,
                          uint8_t *nonce);
extern bool setPassPhrase(const char *passphrase);
extern bool isPassphraseValid();
extern void randomizeBuffer(uint8_t *, int);
#endif
