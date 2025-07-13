#ifndef __CRYPTO_H
#define __CRYPTO_H

#include <stdint.h>
extern void encryptBuffer(const char *password, uint8_t *result, int index,
                          int size);
extern void decryptBuffer(const uint8_t *password, char *result, int index,
                          int size);
extern bool setPassPhrase(const char *passphrase);
extern void randomizeBuffer(uint8_t *, int);
#endif
