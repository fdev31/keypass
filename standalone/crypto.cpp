// NOTE: uses a single buffer, can only handle one at a time !
#include "crypto.h"
#include "ChaCha.h"
#include "Poly1305.h"
#include "constants.h"
#include <BLAKE2s.h>
#include <cstdio>
#include <stdlib.h>
#include <string.h>

// Global variables
static uint8_t passKey[MAX_PASS_LEN];

void randomizeBuffer(uint8_t *buffer, int size) {
  for (size_t i = 0; i < size; i++) {
    buffer[i] = random() % 255;
  }
}

static void deriveNonceFromDigit(int digit, uint8_t *nonce, size_t nonce_len) {
  // Use the same BLAKE2s approach that was used in setPassPhrase
  BLAKE2s blake;

  // Create a seed from the digit
  char seed[16] = {0};
  snprintf(seed, sizeof(seed), "Nonce%d",
           digit); // Convert digit to string with prefix

  // Hash the seed to produce the nonce
  blake.reset(seed, strlen(seed), nonce_len);
  blake.finalize(nonce, nonce_len);
}

bool setPassPhrase(const char *passphrase) {
  BLAKE2s blake;
  uint8_t key[32]; // ChaCha20 uses 32-byte key
  blake.reset(passphrase, strlen(passphrase), 32);
  blake.finalize(key, 32);

  // Only store if needed elsewhere, otherwise can be removed
  memcpy(passKey, key, 32);

  return true;
}

void encryptPassword(const char *password, uint8_t *result, int index) {
  if (!result || !password)
    return;

  // Create a fresh ChaCha instance
  ChaCha encryptChacha(20);

  // Set key (which should have been set by setPassPhrase)
  encryptChacha.setKey(passKey, 32);

  // Derive nonce from index
  uint8_t nonce[12] = {0};
  deriveNonceFromDigit(index, nonce, 12);

  // Store nonce at the beginning of result
  memcpy(result, nonce, 12);

  // Set the IV (nonce)
  encryptChacha.setIV(nonce, 12);

  // Calculate plaintext length
  int plaintext_len = strlen(password) + 1;

  // Store the plaintext length after the nonce
  *((uint32_t *)(result + 12)) = plaintext_len;

  // The actual ciphertext will be stored after the nonce and length
  uint8_t *ciphertext = result + 16;

  // Encrypt the password
  encryptChacha.encrypt(ciphertext, (const uint8_t *)password, plaintext_len);
}

void decryptPassword(const uint8_t *encrypted, char *result, int index) {
  if (!encrypted || !result)
    return;

  // Create a fresh ChaCha instance
  ChaCha decryptChacha(20);

  decryptChacha.setKey(passKey, 32);

  // Extract the nonce from the encrypted data
  const uint8_t *stored_nonce = encrypted;

  // Set the IV (nonce)
  decryptChacha.setIV(stored_nonce, 12);

  // Extract the plaintext length
  uint32_t plaintext_len = *((uint32_t *)(encrypted + 12));

  // Get pointer to the ciphertext
  const uint8_t *ciphertext = encrypted + 16;

  // Decrypt the ciphertext
  decryptChacha.decrypt((uint8_t *)result, ciphertext, plaintext_len);
}
