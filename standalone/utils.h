#include <Arduino.h>
#include <stdint.h>

extern String hexDump(const uint8_t *data, size_t len);
extern void hexParse(const char *hexStr, uint8_t *binData, size_t maxLen);
