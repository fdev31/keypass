#!/bin/env python
import sys

# map: <char> : <code>,<shift pressed>
common_mapping = {
    "\n": (40, 0),
    "\t": (43, 0),
    " ": (44, 0),
    "<": (100, 0),
    ">": (100, 1),
    "/": (84, 0),
    "*": (85, 0),
    "-": (86, 0),
    "+": (87, 0),
}

km = eval(sys.stdin.read())

print("mapping[] = {")
for n in range(0, 255):
    char = chr(n)
    c = km.get(char) or common_mapping.get(char)
    if c is None:
        print("    {0, 0},")
    else:
        print("    {%s, %s}," % c, end="")
        print(f' // "{char if n != 10 else "ENTER"}"')
print("};")