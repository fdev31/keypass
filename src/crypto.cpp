// NOTE: uses a single buffer, can only handle one at a time !
#include "crypto.h"
#include "constants.h"
#ifdef ESP32
#include "esp_random.h"
#endif
#include "utils.h"
#include <BLAKE2s.h>
#include <ChaCha.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static ChaCha chacha(20); // ChaCha20
static uint8_t myhash[32];
static uint8_t tempBuffer[112];

void randomizeBuffer(uint8_t *buffer, int size) {
#ifdef ESP32
  esp_fill_random(buffer, size);
#else
  for (int i = 0; i < size; i++) {
    buffer[i] = random() % 255;
  }
#endif
}

uint8_t *getNonce() {
  static uint8_t nonce[NONCE_LEN];
  randomizeBuffer((uint8_t *)nonce, sizeof(nonce));
  return nonce;
}

bool setPassPhrase(const char *passphrase) {
  BLAKE2s blake;
  blake.reset(passphrase, strlen(passphrase), 32);
  blake.finalize(myhash, 32);
  return chacha.setKey((const uint8_t *)myhash, 32);
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

  chacha.setIV(nonce, 12);

  memcpy(tempBuffer, password, size);
  randomizeBuffer((uint8_t *)(tempBuffer + size), total_size - size);

  chacha.encrypt(result, tempBuffer, blocks * STO_BLOCK_SIZE);
}

void decryptBuffer(const uint8_t *password, char *result, int size,
                   uint8_t *nonce) {
  uint8_t blocks = 0.5 + (size / STO_BLOCK_SIZE);
  chacha.setIV(nonce, 12);
  chacha.decrypt((unsigned char *)result, password, blocks * STO_BLOCK_SIZE);
}
