// NOTE: uses a single buffer, can only handle one at a time !
#include "crypto.h"
#include "constants.h"
#include <BLAKE2s.h>
#include <ChaCha.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static ChaCha chacha(20); // ChaCha20

static const uint8_t *getNonce(int num) {
  static char nonce[12];
  static uint8_t myhash[32];
  static BLAKE2s blake;

  snprintf(nonce, sizeof(nonce), "nonce-%x", num);
  blake.reset(nonce, strlen(nonce), 32);
  blake.finalize(myhash, 32);
  return myhash;
}

uint8_t tempBuffer[112];

void randomizeBuffer(uint8_t *buffer, int size) {
  for (size_t i = 0; i < size; i++) {
    buffer[i] = random() % 255;
  }
}

bool setPassPhrase(const char *passphrase) {
  BLAKE2s blake;
  uint8_t myhash[32];
  blake.reset(passphrase, strlen(passphrase), 32);
  blake.finalize(myhash, 32);
  return chacha.setKey((const uint8_t *)myhash, 32);
}

// TODO: rename to encryptBuffer
void encryptBuffer(const char *password, uint8_t *result, int index, int size) {
  if (!result)
    return;

  if (!password) {
    result[0] = 0;
    return;
  }

  uint8_t blocks = 0.5 + (size / STO_BLOCK_SIZE);
  uint8_t total_size = blocks * STO_BLOCK_SIZE;

  chacha.setIV(getNonce(index), 12);

  memcpy(tempBuffer, password, size);
  randomizeBuffer((uint8_t *)(tempBuffer + size), total_size - size);

  chacha.encrypt(result, tempBuffer, blocks * STO_BLOCK_SIZE);
}

void decryptPassword(const uint8_t *password, char *result, int index,
                     int size) {
  uint8_t blocks = 0.5 + (size / STO_BLOCK_SIZE);
  chacha.setIV(getNonce(index), 12);
  chacha.decrypt((unsigned char *)result, password, blocks * STO_BLOCK_SIZE);
}
