#!/bin/env python
# Sends using CH9389
# Connect to another computer and save the keymap on the other computer using:
# cat > keymap_name

import sys
import serial
import time


AZERTY = True

ser = serial.Serial(port=sys.argv[1], baudrate=9600)

HEADER = [0x57, 0xAB, 0x00]

SCREEN_RES_X = 4096
SCREEN_RES_Y = 4096


def HID(values):
    packet = HEADER + values
    packet.append(sum(packet) % 256)
    return packet


def genMouse(x=0, y=0, left=False, right=False, middle=False, scroll=0, release=False):
    buttons = 0
    if left:
        buttons |= 1
    if right:
        buttons |= 2
    if middle:
        buttons |= 4
    if x < 0:
        x += 255
    if y < 0:
        y += 255
    if scroll:
        scroll = 127 + scroll

    press = HID([0x05, 0x05, 0x01, buttons, x, y, scroll])
    ser.write(press)

    if release:
        time.sleep(0.02)
        release = HID([0x05, 0x05, 0x00, 0x0, 0x00, 0x00, 0x0])
        ser.write(release)


def genKey(
    key=0x04,
    modifiers=0,
):
    press = HID([0x02, 0x08, modifiers, 0x00, key, 0x00, 0x00, 0x00, 0x00, 0x00])
    ser.write(press)
    time.sleep(0.02)
    rel = HID([0x02, 0x08, modifiers, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00])
    ser.write(rel)


def printNumber(num):
    for c in str(num):
        v = ord(c) - ord("1")
        if v < 0:
            v = 9
        genKey(30 + v, modifiers=2 if AZERTY else 0)
        time.sleep(0.02)


altgr_keys = {48, 45, 46, 53}
for n in range(30, 40):
    altgr_keys.add(n)


def typeKey(n):
    genKey(n)
    time.sleep(0.1)
    genKey(n, modifiers=2)
    time.sleep(0.1)
    if n in altgr_keys:
        genKey(n, modifiers=0b1000000)
        time.sleep(0.1)
    genKey(44)
    time.sleep(0.1)
    printNumber(n)
    time.sleep(0.1)
    genKey(0x28)
    time.sleep(0.1)


def main():
    typeKey(100)
    time.sleep(0.05)
    # Normal
    for n in range(4, 57):
        if 40 <= n <= 44:
            continue
        # 40=RETURN
        # 41=DEL
        # 42=BS
        # 43=TAB
        # 44=SPACE
        typeKey(n)
        time.sleep(0.05)

    # 4 operators
    for n in range(84, 88):
        typeKey(n)
        time.sleep(0.05)


def jiggle_mouse():
    while True:
        genMouse(x=1)
        time.sleep(0.1)
        genMouse(x=-1)
        time.sleep(1)


main()
jiggle_mouse()
