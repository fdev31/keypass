#ifndef _IMPORTEXPORT_H
#define _IMPORTEXPORT_H

#include "streamadapter.h"
#include <stdint.h>

void dumpSinglePassword(StringStreamAdapter &stream, const char *label,
                        const char *password, const char layout,
                        uint8_t *nonce);
bool parseSinglePassword(const char *rawdata, char *label, char *password,
                         int *layout);

#endif // _IMPORTEXPORT_H
