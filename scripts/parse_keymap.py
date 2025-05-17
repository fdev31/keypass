#!/bin/env python
import sys
from pprint import pprint
import fileinput

results: list[dict[str, tuple[int, int]]] = []
current_file = None

code_map = {
    0: 0,
    1: 2,
    2: 64,
}

for filename in sys.argv[1:]:
    collected: dict[str, tuple[int, int]] = {}
    for line in open(filename).readlines():
        try:
            letters, code_str = line.strip().split(" ")
            code = int(code_str)
        except ValueError:
            sys.stderr.write(f"skipping bogus: <{line.strip()}>\n")
            continue
        code = int(code)
        for i, c in enumerate(letters):
            to_skip = False
            if c in collected:
                if collected[c] != (code, i):
                    if collected[c][1] > i:
                        sys.stderr.write(
                            f"Replacing {c} using code {(code, code_map[i])} - already defined as {collected[c]} !\n"
                        )
                    else:
                        sys.stderr.write(
                            f"Skipping duplicate for {c} using code {code} - already defined as {collected[c]} !\n"
                        )
                        to_skip = True

            if not to_skip:
                collected[c] = (code, code_map[i] if i else 0)
    results.append(collected)

pprint(results)
