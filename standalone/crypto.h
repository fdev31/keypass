#ifndef __CRYPTO_H
#define __CRYPTO_H

#include <stdint.h>
extern void encryptPassword(const char *password, uint8_t *result, int index);
extern void decryptPassword(const uint8_t *password, char *result, int index);
extern bool setPassPhrase(const char *passphrase);
extern void randomizeBuffer(uint8_t *, int);
#endif
