#!/bin/env python
import sys
from pprint import pprint
import fileinput

collected = {}

for line in fileinput.input():
    try:
        letters, code = line.strip().split(" ")
    except ValueError:
        sys.stderr.write(f"skipping bogus: <{line.strip()}>\n")
        continue
    code = int(code)
    for i, c in enumerate(letters):
        if c in collected:
            if collected[c] != (code, i):
                sys.stderr.write(f"Warning, duplicate for {c} using code {code} - already defined as {collected[c]} !\n")
            else:
                sys.stderr.write(f"Warning, duplicate entry for {c} using code {code}\n")
    collected[letters[0]] = (code, 0)
    collected[letters[1]] = (code, 1)

pprint(collected)
