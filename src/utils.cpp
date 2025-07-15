#include "utils.h"
#include "streamadapter.h"
#include <stdint.h>
#include <stdio.h>

const char *mkEntryName(int num) {
  static char name[16];
  snprintf(name, sizeof(name), "p%d", num);
  return name;
}

void hexDump(StringStreamAdapter &stream, const uint8_t *data, size_t len) {
  char hexChars[3]; // Two characters for the hex value plus null terminator

  for (size_t i = 0; i < len; i++) {
    // Convert each byte to a two-character hex representation
    sprintf(hexChars, "%02X", data[i]);
    stream.write((const char *)hexChars);
  }
}

// Parse a hex string back into binary data
void hexParse(const char *hexStr, uint8_t *binData, size_t maxLen) {
  size_t len = 0;
  while (*hexStr && *(hexStr + 1) && len < maxLen) {
    char highNibble = *hexStr++;
    char lowNibble = *hexStr++;

    uint8_t value = 0;
    if (highNibble >= '0' && highNibble <= '9')
      value = (highNibble - '0') << 4;
    else if (highNibble >= 'A' && highNibble <= 'F')
      value = (highNibble - 'A' + 10) << 4;
    else if (highNibble >= 'a' && highNibble <= 'f')
      value = (highNibble - 'a' + 10) << 4;

    if (lowNibble >= '0' && lowNibble <= '9')
      value |= (lowNibble - '0');
    else if (lowNibble >= 'A' && lowNibble <= 'F')
      value |= (lowNibble - 'A' + 10);
    else if (lowNibble >= 'a' && lowNibble <= 'f')
      value |= (lowNibble - 'a' + 10);

    binData[len++] = value;
  }
}
