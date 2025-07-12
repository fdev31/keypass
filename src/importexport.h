#include <Arduino.h>

extern String dumpSinglePassword(const char *label, const char *password,
                                 int password_size, const char layout,
                                 const unsigned char version, int index);

extern bool parseSinglePassword(const char *rawdata, char *label,
                                char *password, int *password_size,
                                char *layout, unsigned char *version,
                                int index);
