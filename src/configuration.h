#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

const unsigned long SHUTDOWN_TIMEOUT = 2 * 60 * 1000; // minutes in ms
// FYI The SSID can't have a space in it.
#define DEFAULT_WIFI_SSID "KeyPass"
// At least 8 chars
#define DEFAULT_WIFI_PASSWORD "12345678" // set to NULL for no password

#define ENABLE_GRAPHICS true
#define USE_EEPROM_API 0
#define USE_CH9329 1
#define FLIP_SCREEN 1
#endif
