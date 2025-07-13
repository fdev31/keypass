#include <Arduino.h>

extern String dumpSinglePassword(const char *label, const char *password,
                                 const char layout, const unsigned char version,
                                 int index);

extern bool parseSinglePassword(const char *rawdata, char *label,
                                char *password, char *layout,
                                unsigned char *version, int index);
