#ifndef _UTILS_H
#define _UTILS_H

#include "streamadapter.h"
#include <stddef.h>
#include <stdint.h>

const char *mkEntryName(int num);
void hexDump(StringStreamAdapter &stream, const uint8_t *data, size_t len);
void hexParse(const char *hexStr, uint8_t *binData, size_t maxLen);

#endif // _UTILS_H
