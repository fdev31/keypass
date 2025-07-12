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

String dumpSinglePassword(const char *label, const char *password,
                          int password_size, const char layout,
                          const unsigned char version, int index) {

  char buffer[DUMP_LENGTH];

  buffer[0] = version;
  buffer[1] = layout;
  int sz = strlen(label) + 1;
  int offset = 2;
  memcpy(buffer + offset, label, sz);
  offset += MAX_NAME_LEN;
  memcpy(buffer + offset, password, MAX_PASS_LEN);
  offset += MAX_PASS_LEN;

  // printf("XXXX=>");
  // hexDump((const uint8_t *)buffer, DUMP_LENGTH);
  encryptBuffer((const char *)buffer + 2, (uint8_t *)buffer + 2, index,
                DUMP_LENGTH - 2);

  return hexDump((const uint8_t *)buffer, DUMP_LENGTH);
}
bool parseSinglePassword(const char *rawdata, char *label, char *password,
                         int *password_size, char *layout,
                         unsigned char *version, int index) {

  // Calculate the binary data size needed
  const size_t binarySize = DUMP_LENGTH;
  uint8_t buffer[DUMP_LENGTH];
  char tmp_buffer[DUMP_LENGTH - 2];

  // Convert hex dump to binary
  hexParse(rawdata, buffer, binarySize);

  // Extract metadata
  *version = buffer[0];
  *layout = buffer[1];

  // Decrypt the data starting after version and layout
  decryptPassword((const uint8_t *)buffer + 2, tmp_buffer, index,
                  DUMP_LENGTH - 2);

  // Extract label (name)
  strncpy(label, tmp_buffer, MAX_NAME_LEN);

  // Extract password (which follows the label)
  memcpy(password, (const char *)tmp_buffer + MAX_NAME_LEN, MAX_PASS_LEN);

  return true;
}
