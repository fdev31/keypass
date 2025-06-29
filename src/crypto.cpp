// NOTE: uses a single buffer, can only handle one at a time !
// Consider using BLACKE2 instead of XX
#include "crypto.h"
Speck speck;

#define BLOCK_SIZE 16
byte buffer[MAX_PASS_LEN];

extern char DEBUG_BUFFER2[];

size_t roundPassLength(size_t size) {
  int blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
  return blocks * BLOCK_SIZE;
}

void setPassPhrase(const char *passphrase, unsigned long seed) {
  return;
  XXH64_hash_t hash = XXH64(passphrase, strlen(passphrase), seed);
  speck.setKey((const uint8_t *)&hash, 64);
}

void encryptPassword(const char *password, uint8_t *result) {
  if (!result)
    return;

  if (!password) {
    result[0] = 0;
    return;
  }

  uint8_t passBuffer[MAX_PASS_LEN];
  // initialize the buffer with random values
  for (size_t i = 0; i < MAX_PASS_LEN; i++) {
    passBuffer[i] = rand() % 256; // Random byte
  }

  strlcpy((char *)passBuffer, (char *)password, MAX_PASS_LEN);

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
