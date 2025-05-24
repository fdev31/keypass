#!/bin/env python
import sys

# map: <char> : <code>,<shift pressed>
common_mapping = {
    "\n": (40, 0),
    "\t": (43, 0),
    " ": (44, 0),
    "<": (100, 0),
    ">": (100, 2),
    "/": (84, 0),
    "*": (85, 0),
    "-": (86, 0),
    "+": (87, 0),
}

keymaps = eval(sys.stdin.read())
first_keymap = keymaps[0]
last_keymap = keymaps[-1]

# assert len(first_keymap) == len(last_keymap), "Keymaps must be the same length"

print("#ifndef __CUSTOM_KEYMAP")
print("#define __CUSTOM_KEYMAP")
print("#include <stdint.h>")
print("static uint8_t KBD_MAP[][255][2] = {")
for km in keymaps:
    print("{")
    for n in range(0, 255):
        char = chr(n)
        c = km.get(char) or common_mapping.get(char)
        if c is None:
            print("    {0, 0},")
        else:
            print("    {%s, %s}," % c, end="")
            print(f' // "{char if n != 10 else "ENTER"}"')
    print("}", end="")
    if km is not last_keymap:
        print(",", end="")
print("};")
print("#endif")
