#include "configuration.h"
#include "keymap.h"
#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#if not USE_CH9329

class MockHIDDevice {
public:
  void sendKey(uint8_t key, uint8_t modifiers);
};

void MockHIDDevice::sendKey(uint8_t key, uint8_t modifiers) {}

MockHIDDevice dev;
#else
static const uint8_t HEADER[] = {0x57, 0xAB, 0x00};

static void sendHID(uint8_t values[], int length) {
  uint8_t packet_sz = length + sizeof(HEADER);
  uint8_t packet[packet_sz + 1]; // +1 for checksum

  memcpy(packet, HEADER, sizeof(HEADER));
  memcpy(packet + sizeof(HEADER), values, length);

  // Initialize checksum to 0
  packet[packet_sz] = 0;

  for (int i = 0; i < packet_sz; i++) {
    packet[packet_sz] += packet[i];
  }

  Serial.write(packet, packet_sz + 1);
}
#endif

void sendHIDInit() {
  // Set manufacturer string
  uint8_t manufacturer[] = {0x0B, // Command (0x0B for setting USB strings)
                            0x00, // String index (0 for manufacturer)
                            0x09, // Length of string data
                            'K',  'e', 'y', 'P', 'a', 's', 's'};
  sendHID(manufacturer, sizeof(manufacturer));
  delay(200);

  // Set product string
  uint8_t product[] = {0x0B, // Command (0x0B for setting USB strings)
                       0x01, // String index (1 for product)
                       0x09, // Length of string data
                       'K',  'e', 'y', 'P', 'A', 'S', 'S'};
  sendHID(product, sizeof(product));
  delay(200);

  // Save configuration
  uint8_t save[] = {
      0x0C, // Save command
      0x00  // Length (no data for save command)
  };
  sendHID(save, sizeof(save));
  delay(200);

  // Reset the device
  uint8_t reset[] = {
      0x0F, // Reset command
      0x00  // Length (no data for reset command)
  };
  sendHID(reset, sizeof(reset));
  delay(200);
}

static void genKey(uint8_t key, uint8_t modifiers) {
#if USE_CH9329
  // Step 1: Press modifier first (if any)
  if (modifiers != 0) {
    uint8_t modPress[] = {0x02, 0x08, modifiers, 0x00, 0x00,
                          0x00, 0x00, 0x00,      0x00, 0x00};
    sendHID(modPress, sizeof(modPress));
    delay(10); // Small delay after pressing modifier
  }

  // Step 2: Press key with modifier (if there's a key to press)
  if (key != 0) {
    uint8_t keyPress[] = {0x02, 0x08, modifiers, 0x00, key,
                          0x00, 0x00, 0x00,      0x00, 0x00};
    sendHID(keyPress, sizeof(keyPress));
    delay(20); // Key press duration
  }

  // Step 3: Release key but maintain modifier
  if (key != 0) {
    uint8_t keyRelease[] = {0x02, 0x08, modifiers, 0x00, 0x00,
                            0x00, 0x00, 0x00,      0x00, 0x00};
    sendHID(keyRelease, sizeof(keyRelease));
    delay(10); // Small delay after releasing key
  }

  // Step 4: Release modifier (if any was pressed)
  if (modifiers != 0) {
    uint8_t modRelease[] = {0x02, 0x08, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00};
    sendHID(modRelease, sizeof(modRelease));
  }
#else
  dev.sendKey(key, modifiers);
#endif
}

void sendFunctionKey(uint8_t fKeyNum) {
  // F1-F12 have USB HID key codes 0x3A to 0x45
  uint8_t fKeyCode = fKeyNum ? 0x3A + (fKeyNum - 1) : 0x43;
  return genKey(fKeyCode, 0);
}

// Function to lookup keycode and modifier for a character
static int lookupKeyMapping(char c, KeyboardLayout layout, uint8_t *keycode,
                            uint8_t *modifier) {
  const KeyMapping *currentLayout = KEYBOARD_LAYOUTS[layout];

  // Search through the layout table
  for (int i = 0; currentLayout[i].ascii != 0; i++) {
    if (currentLayout[i].ascii == c) {
      *keycode = currentLayout[i].keycode;
      *modifier = currentLayout[i].modifier;
      return 1; // Found
    }
  }

  // Character not found
  *keycode = 0;
  *modifier = 0;
  return 0;
}

// Function to send a key based on character and layout
void sendKeymap(char c, int layout) {
  uint8_t keycode, modifier;

  if (lookupKeyMapping(c, (KeyboardLayout)layout, &keycode, &modifier)) {
    genKey(keycode, modifier);
  }
  // If character not found, nothing happens
}

void sendKey(char c, bool useFxKeys) {
  if (useFxKeys) {
    if (c == '0') {
      sendFunctionKey(10); // F10 for '0'
    } else if (c >= '1' && c <= '9') {
      sendFunctionKey(c - '0'); // Convert '1'-'9' to 1-9 for F1-F9
    } else if (c == 'a' || c == 'A') {
      sendFunctionKey(11); // F11 for 'A'
    } else if (c == 'b' || c == 'B') {
      sendFunctionKey(12); // F12 for 'B'
    }
  } else {
    // Use the normal keymap
    sendKeymap(c, KBLAYOUT_US);
  }
}
