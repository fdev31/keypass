#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

const unsigned long AUTOSLEEP_TIMEOUT = 100 * 1000; // auto-sleep delay
const unsigned long SLEEP_WAKE_TIME = 10 * 1000;    // busy period in sleep mode
const unsigned long SLEEP_TIME = 500; // sleep period in sleep mode
// Max wifi clients, keep it low for security, or higher for dev
#define MAX_CLIENTS 1
#define WIFI_CHANNEL 6
// FYI The SSID can't have a space in it.
#define DEFAULT_WIFI_SSID "KeyPass"
// At least 8 chars
#define DEFAULT_WIFI_PASSWORD "12345678" // set to NULL for no password

#define ENABLE_GRAPHICS true
#define USE_EEPROM_API 0
#define USE_CH9329 1
#define FLIP_SCREEN 1
#define DEBUG 0
#endif
