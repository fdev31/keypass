#include "constants.h"
#include "crypto.h"
#include "utils.h"
#include <Arduino.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>

String dumpSinglePassword(const char *label, const char *password,
                          int password_size, const char layout,
                          const unsigned char version, int index) {

  char buffer[DUMP_LENGTH];

  char salt = random() % 255; // Generate a random salt byte
  char *ptr = buffer;
  *ptr++ = salt;
  *ptr++ = salt ^ version;
  *ptr++ = salt ^ layout;

  int sz = strlen(label) + 1;
  memcpy(ptr, label, sz);
  ptr += MAX_NAME_LEN;

  randomizeBuffer((uint8_t *)buffer + CRYPTO_HEADER_SIZE + sz,
                  DUMP_LENGTH - CRYPTO_HEADER_SIZE - sz);

  memcpy(ptr, password, MAX_PASS_LEN);

  // fprintf(stderr, "DEBUG=>%s#\n",
  //         hexDump((const uint8_t *)buffer, DUMP_LENGTH).c_str());

  encryptBuffer((const char *)buffer, (uint8_t *)buffer, index, DUMP_LENGTH);

  return hexDump((const uint8_t *)buffer, DUMP_LENGTH);
}

bool parseSinglePassword(const char *rawdata, char *label, char *password,
                         int *password_size, char *layout,
                         unsigned char *version, int index) {

  // Calculate the binary data size needed
  uint8_t buffer[DUMP_LENGTH];

  // Convert hex dump to binary
  hexParse(rawdata, buffer, DUMP_LENGTH);

  decryptPassword((const uint8_t *)buffer, (char *)buffer, index, DUMP_LENGTH);

  char salt = buffer[0];

  *version = buffer[1] ^ salt;
  *layout = buffer[2] ^ salt;

  // Decrypt the data starting after version and layout

  // Extract label (name)
  strncpy(label, (const char *)buffer + CRYPTO_HEADER_SIZE, MAX_NAME_LEN);

  // Extract password (which follows the label)
  memcpy(password, (const char *)buffer + META_SIZE, MAX_PASS_LEN);

  return true;
}
