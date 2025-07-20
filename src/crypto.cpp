// NOTE: uses a single buffer, can only handle one at a time !
#include "crypto.h"
#include "constants.h"
#ifdef ESP32
#include "esp_random.h"
#endif
#include "utils.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sodium.h>

static uint8_t myhash[crypto_generichash_BYTES];
static uint8_t tempBuffer[112];

void randomizeBuffer(uint8_t *buffer, int size) {
#ifdef ESP32
  esp_fill_random(buffer, size);
#else
  randombytes_buf(buffer, size);
#endif
}

uint8_t *getNonce() {
  static uint8_t nonce[NONCE_LEN];
  randomizeBuffer((uint8_t *)nonce, sizeof(nonce));
  return nonce;
}

bool setPassPhrase(const char *passphrase) {
  if (!passphrase) {
    return false;
  }

  crypto_generichash(myhash, sizeof(myhash), (const unsigned char *)passphrase,
                     strlen(passphrase), NULL, 0);
  return true;
}

void encryptBuffer(const char *password, uint8_t *result, int size,
                   uint8_t *nonce) {
  if (!result)
    return;

  if (!password) {
    result[0] = 0;
    return;
  }

  uint8_t blocks = 0.5 + (size / STO_BLOCK_SIZE);
  uint8_t total_size = blocks * STO_BLOCK_SIZE;

  // Copy password to temp buffer and pad with random data
  memcpy(tempBuffer, password, size);
  randomizeBuffer((uint8_t *)(tempBuffer + size), total_size - size);

  // Generate keystream and XOR with the data
  crypto_stream_chacha20_xor(result, tempBuffer, total_size, nonce, myhash);
}

void decryptBuffer(const uint8_t *password, char *result, int size,
                   uint8_t *nonce) {
  uint8_t blocks = 0.5 + (size / STO_BLOCK_SIZE);
  uint8_t total_size = blocks * STO_BLOCK_SIZE;

  // Decrypt by XORing with the same keystream
  crypto_stream_chacha20_xor((unsigned char *)result, password, total_size,
                             nonce, myhash);
}
