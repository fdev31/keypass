
#include "importexport.h"
#include "Arduino.h"
#include "constants.h"
#include "crypto.h"
#include "streamadapter.h"
#include "utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>

// Buffer for formatting a single password entry
// Declared static to avoid recreating it on each call
static char passwordBuffer[(DUMP_LENGTH + UNENCRYPTED_DATA_LENGTH) * 2 + 1];

void dumpSinglePassword(StringStreamAdapter &stream, const char *label,
                        const char *password, const char layout,
                        uint8_t *nonce) {
  // Clear the buffer to ensure no stale data remains
  memset(passwordBuffer, 0, DUMP_LENGTH + UNENCRYPTED_DATA_LENGTH);

  char salt = random() % 255; // Generate a random salt byte

  // Set up header data
  passwordBuffer[0] = FORMAT_VERSION; // Version byte
  memcpy(passwordBuffer + 1, nonce, NONCE_LEN);

  // Set up encrypted section
  char *encryptedStart = passwordBuffer + UNENCRYPTED_DATA_LENGTH;
  encryptedStart[0] = salt;
  encryptedStart[1] = salt ^ layout;

  // Copy label with null terminator
  int labelLength = strlen(label) + 1;
  memcpy(encryptedStart + CRYPTO_HEADER_SIZE, label, labelLength);

  // Fill unused space with random data for security
  randomizeBuffer((uint8_t *)encryptedStart + CRYPTO_HEADER_SIZE + labelLength,
                  DUMP_LENGTH - CRYPTO_HEADER_SIZE - labelLength);

  // Copy password to the correct position
  memcpy(encryptedStart + CRYPTO_HEADER_SIZE + MAX_NAME_LEN, password,
         MAX_PASS_LEN);

  // Encrypt the data section
  encryptBuffer(encryptedStart, (uint8_t *)encryptedStart, DUMP_LENGTH, nonce);

  // Output as hex to the stream
  hexDump(stream, (const uint8_t *)passwordBuffer,
          DUMP_LENGTH + UNENCRYPTED_DATA_LENGTH);
}

bool parseSinglePassword(const char *rawdata, char *label, char *password,
                         int *layout) {
  static uint8_t buffer[DUMP_LENGTH + UNENCRYPTED_DATA_LENGTH];
  uint8_t *nonce = buffer + 1;

  hexParse(rawdata, buffer, DUMP_LENGTH + UNENCRYPTED_DATA_LENGTH);

  uint8_t *encryptedSection = buffer + UNENCRYPTED_DATA_LENGTH;

  decryptBuffer(encryptedSection, (char *)encryptedSection, DUMP_LENGTH, nonce);

  if (buffer[0] == 1) { // FORMAT_VERSION == 1
    *layout =
        encryptedSection[1] ^ encryptedSection[0]; // Decrypt layout using salt

    // Copy label and password to output buffers
    strlcpy(label, (const char *)(encryptedSection + CRYPTO_HEADER_SIZE),
            MAX_NAME_LEN);
    memcpy(password,
           (const char *)(encryptedSection + CRYPTO_HEADER_SIZE + MAX_NAME_LEN),
           MAX_PASS_LEN);

    return true;
  }

  return false; // Unsupported format version
}
