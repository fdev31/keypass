// NOTE: uses a single buffer, can only handle one at a time !
// Consider using BLACKE2 instead of XX
#include "crypto.h"
#include "constants.h"
Speck speck;

uint8_t buffer[MAX_PASS_LEN];

extern char DEBUG_BUFFER2[];

void randomizeBuffer(uint8_t *buffer, int size) {
  for (size_t i = 0; i < size; i++) {
    buffer[i] = random() % 255;
  }
}

bool setPassPhrase(const char *passphrase, unsigned long seed) {
  XXH64_hash_t hash = XXH64(passphrase, strlen(passphrase), seed);
  return speck.setKey((const uint8_t *)&hash, 64);
}

void encryptPassword(const char *password, uint8_t *result) {
  if (!result)
    return;

  if (!password) {
    result[0] = 0;
    return;
  }

  uint8_t passBuffer[MAX_PASS_LEN];
  randomizeBuffer(passBuffer, MAX_PASS_LEN);

  int maxlen = strlen(password) + 1;
  if (maxlen > MAX_PASS_LEN) {
    maxlen = MAX_PASS_LEN;
  }

  strlcpy((char *)passBuffer, (char *)password, maxlen);

  // Process each 16-byte block
  for (size_t i = 0; i < STORED_PASSWD_BLOCKS; i++) {
    int offset = i * BLOCK_SIZE;
    speck.encryptBlock(result + offset, passBuffer + offset);
  }
}

void decryptPassword(const uint8_t *password, char *result) {
  // Process each 16-byte block
  for (size_t i = 0; i < STORED_PASSWD_BLOCKS; i++) {
    int offset = i * BLOCK_SIZE;
    // Decrypt the block
    speck.decryptBlock((uint8_t *)result + offset, password + offset);
  }
}
