#include "configuration.h"
#include "keymap.h"
#include <Arduino.h>
#include <stdint.h>
#include <string.h>

#if not USE_CH9329

class MockHIDDevice {
public:
  void sendKey(uint8_t key, uint8_t modifiers);
};

void MockHIDDevice::sendKey(uint8_t key, uint8_t modifiers) {}

MockHIDDevice dev;
#else
static const uint8_t HEADER[] = {0x57, 0xAB, 0x00};

void sendHID(uint8_t values[], int length) {
  uint8_t packet[length + 1];
  uint8_t packet_sz = length + sizeof(HEADER);
  memcpy(packet, HEADER, sizeof(HEADER));
  memcpy(packet + sizeof(HEADER), values, length);
  packet[packet_sz] = 0;

  for (int i = 0; i < packet_sz; i++) {
    packet[packet_sz] += packet[i];
  }

  Serial.write(packet, packet_sz + 1);
}
#endif

void genKey(uint8_t key, uint8_t modifiers) {
#if USE_CH9329
  uint8_t press[] = {0x02, 0x08, modifiers, 0x00, key,
                     0x00, 0x00, 0x00,      0x00, 0x00};
  uint8_t rel[] = {0x02, 0x08, modifiers, 0x00, 0x00,
                   0x00, 0x00, 0x00,      0x00, 0x00};
  sendHID(press, sizeof(press));
  delay(20);
  sendHID(rel, sizeof(rel));
#else
  dev.sendKey(key, modifiers);
#endif
}

void sendFunctionKey(uint8_t fKeyNum) {
  // F1-F12 have USB HID key codes 0x3A to 0x45
  uint8_t fKeyCode = fKeyNum ? 0x3A + (fKeyNum - 1) : 0x43;
  return genKey(fKeyCode, 0);
}

void sendKey(char c, bool useFxKeys) {
  if (useFxKeys) {
    return sendFunctionKey(c - '0');
  } else {
    // Use the normal keymap
    genKey(KBD_MAP[0][(int)c][0], KBD_MAP[0][(int)c][1]);
  }
}

void sendKeymap(char c, int keymap) {
  genKey(KBD_MAP[keymap][(int)c][0], KBD_MAP[keymap][(int)c][1]);
}
