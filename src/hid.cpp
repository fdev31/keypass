#include "keymap.h"
#include <Arduino.h>
#include <string.h>
#include <stdint.h>

const uint8_t HEADER[] = {0x57, 0xAB, 0x00};

void sendHID(uint8_t values[], int length) {
    uint8_t packet[length + 1];
    memcpy(packet, HEADER, sizeof(HEADER));
    memcpy(packet + sizeof(HEADER), values, length);
    packet[length + sizeof(HEADER)] = 0;

    for (int i = 0; i < length + sizeof(HEADER); i++) {
        packet[length + sizeof(HEADER)] += packet[i];
    }

    Serial.write(packet, length + sizeof(HEADER) + 1);
}

void genKey(uint8_t key, uint8_t modifiers) {
    uint8_t press[] = {0x02, 0x08, modifiers, 0x00, key,
        0x00, 0x00, 0x00,      0x00, 0x00};
    uint8_t rel[] = {0x02, 0x08, modifiers, 0x00, 0x00,
        0x00, 0x00, 0x00,      0x00, 0x00};

    sendHID(press, sizeof(press));
    delay(20);
    sendHID(rel, sizeof(rel));
}

void sendKey(char c) { genKey(KBD_MAP[(int)c][0], KBD_MAP[(int)c][1]); }
