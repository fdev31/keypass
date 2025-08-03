#ifndef __CUSTOM_KEYMAP
#define __CUSTOM_KEYMAP
#include <stdint.h>

// Define a struct for each key mapping
typedef struct {
    uint8_t ascii;     // ASCII code
    uint8_t keycode;   // HID key code
    uint8_t modifier;  // Modifier key
} KeyMapping;

// Enum for supported layouts
typedef enum {
    KBLAYOUT_FR = 0,
    KBLAYOUT_US = 1
} KeyboardLayout;


extern const KeyMapping* KEYBOARD_LAYOUTS[];
#endif // __CUSTOM_KEYMAP
