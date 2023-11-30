#!/bin/env python
# Sends using CH9389

import sys
import serial
import time


AZERTY = True

ser = serial.Serial(port=sys.argv[1], baudrate=9600)


def genKey(
    key=0x04,
    modifiers=0,
):
    press = [0x57, 0xAB, 0x00, 0x02, 0x08, modifiers, 0x00, key, 0x00, 0x00, 0x00, 0x00, 0x00]
    press.append(sum(press) % 256)
    ser.write(press)
    time.sleep(0.02)
    rel = [0x57, 0xAB, 0x00, 0x02, 0x08, modifiers, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00]
    rel.append(sum(rel) % 256)
    ser.write(rel)


def printNumber(num):
    for c in str(num):
        v = ord(c) - ord("1")
        if v < 0:
            v = 9
        genKey(30 + v, modifiers=2 if AZERTY else 0)
        time.sleep(0.02)


def typeKey(n):
    genKey(n)
    time.sleep(0.1)
    genKey(n, modifiers=2)
    time.sleep(0.1)
    genKey(44)
    time.sleep(0.1)
    printNumber(n)
    time.sleep(0.1)
    genKey(0x28)
    time.sleep(0.1)


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
