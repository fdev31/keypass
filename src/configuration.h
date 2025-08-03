#include "constants.h"
#include "version.h"
#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#define SDA_PIN 5
#define SCL_PIN 6

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

#define ENABLE_FULL_ENCRYPTION 1 // also encrypt names
#define ENABLE_BLUETOOTH true
#define ENABLE_HTTP true
#define ENABLE_GRAPHICS true

#ifndef HW_TYPE
#define HW_TYPE 1
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#pragma message("*** HW_TYPE is set to " TOSTRING(HW_TYPE))

#if HW_TYPE == 0 // standard device
#define BUGGY_OFFSET_X 0
#define BUGGY_OFFSET_Y 0
#elif HW_TYPE == 1         // custom device A
#define BUGGY_DISPLAY true // Some devices have a non-standard control
#define BUGGY_OFFSET_X 30
#define BUGGY_OFFSET_Y 12
#elif HW_TYPE == 2         // custom device B
#define BUGGY_DISPLAY true // Some devices have a non-standard control
#define BUGGY_OFFSET_X 28
#define BUGGY_OFFSET_Y 24
#endif

#define DEBUG 0

// WARN: deprecated options (non default will break)
#define USE_EEPROM_API 0
#define USE_CH9329 1
#define FLIP_SCREEN 1
#endif
