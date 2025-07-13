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

  char buffer[DUMP_LENGTH + 1];

  char salt = random() % 255; // Generate a random salt byte
  char *ptr = buffer;
  *ptr++ = version;
  *ptr++ = salt;
  *ptr++ = salt ^ layout;

  int labelLength = strlen(label) + 1;
  memcpy(ptr, label, labelLength);
  ptr += MAX_NAME_LEN;

  randomizeBuffer((uint8_t *)buffer + CRYPTO_HEADER_SIZE + labelLength + 1,
                  DUMP_LENGTH - CRYPTO_HEADER_SIZE - labelLength - 1);

  memcpy(ptr, password, MAX_PASS_LEN);

  // fprintf(stderr, "DEBUG=>%s#\n",
  //         hexDump((const uint8_t *)buffer, DUMP_LENGTH).c_str());

  encryptBuffer(buffer + 1, (uint8_t *)buffer + 1, index, DUMP_LENGTH);

  return hexDump((const uint8_t *)buffer, DUMP_LENGTH + 1);
}

bool parseSinglePassword(const char *rawdata, char *label, char *password,
                         int *password_size, char *layout,
                         unsigned char *version, int index) {

  uint8_t buffer[DUMP_LENGTH + 1];
  uint8_t *ptr = buffer;

  hexParse(rawdata, buffer, DUMP_LENGTH + 1);
  ptr++;

  decryptBuffer(ptr, (char *)ptr, index, DUMP_LENGTH);
  *version = buffer[0];
  switch (*version) {
  case 1:
    *layout = ptr[1] ^ ptr[0];
    ptr += CRYPTO_HEADER_SIZE;

    strncpy(label, (const char *)ptr, MAX_NAME_LEN);
    ptr += MAX_NAME_LEN;

    memcpy(password, (const char *)ptr, MAX_PASS_LEN);
  }

  return true;
}
