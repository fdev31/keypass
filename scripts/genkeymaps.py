#!/usr/bin/env python3

import os
import sys
import glob
from pathlib import Path


def parse_keymap_file(filename):
    layout_name = Path(
        filename
    ).stem.upper()  # Get filename without extension, uppercase
    results: list[dict[str, tuple[int, int]]] = []
    current_file = None

    code_map = {
        0: 0,
        1: 2,
        2: 64,
    }

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

    def convert_res(key_mapping):
        # convert mappings "char: (code, modifier)" into ""(char, code, modifier)""
        return [(k, v[0], v[1]) for k, v in key_mapping.items()]

    return layout_name, convert_res(results[0])


def generate_keymap_header(layout_data):
    """Generate the C header file from the parsed layout data"""

    header = """#ifndef __CUSTOM_KEYMAP
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
"""

    # Generate the enum values
    enum_values = []
    for i, (layout_name, _) in enumerate(layout_data):
        enum_values.append(f"    KBLAYOUT_{layout_name} = {i}")

    header += ",\n".join(enum_values) + "\n} KeyboardLayout;\n\n"

    # Generate each layout array
    for layout_name, mappings in layout_data:
        header += f"// {layout_name} keyboard layout\n"
        header += f"static const KeyMapping LAYOUT_{layout_name}[] = {{\n"

        # Add special characters
        header += "    {'\\t', 43, 0},  // Tab\n"
        header += "    {'\\n', 40, 0},  // Enter\n"
        header += "    {' ', 44, 0},    // Space\n"

        # Add all mappings
        for char, keycode, modifier in mappings:
            if char not in " \t\n\\":
                header += f"    {{{ord(char)}, {keycode}, {modifier}}}, // {char}\n"
            else:
                header += f"    {{{ord(char)}, {keycode}, {modifier}}},"

        # Add sentinel value
        header += "    {0, 0, 0}  // End marker\n};\n\n"

    # Generate the layouts array
    header += "// Array of pointers to keyboard layouts for easy access\n"
    header += "static const KeyMapping* KEYBOARD_LAYOUTS[] = {\n"
    layout_pointers = []
    for layout_name, _ in layout_data:
        layout_pointers.append(f"    LAYOUT_{layout_name}")
    header += ",\n".join(layout_pointers) + "\n};\n\n"

    header += "\n#endif // __CUSTOM_KEYMAP\n"
    return header


def main():
    """Main function to process command line arguments and generate the header file"""
    if len(sys.argv) < 2:
        print("Usage: python generate_keymap.py <keymap_dir_or_files>")
        print("Example: python generate_keymap.py keymaps/")
        print("         python generate_keymap.py keymaps/fr keymaps/us")
        return 1

    layout_data = []

    # Process each input argument (directory or file)
    for path_arg in sys.argv[1:]:
        if os.path.isdir(path_arg):
            # If directory, process all text files in it
            files = glob.glob(os.path.join(path_arg, "*"))
            for file in files:
                if os.path.isfile(file):
                    try:
                        layout_name, mappings = parse_keymap_file(file)
                        layout_data.append((layout_name, mappings))
                        print(
                            f"Processed {file}: {len(mappings)} mappings",
                            file=sys.stderr,
                        )
                    except Exception as e:
                        print(f"Error processing {file}: {str(e)}", file=sys.stderr)
        elif os.path.isfile(path_arg):
            # Process single file
            try:
                layout_name, mappings = parse_keymap_file(path_arg)
                layout_data.append((layout_name, mappings))
                print(
                    f"Processed {path_arg}: {len(mappings)} mappings", file=sys.stderr
                )
            except Exception as e:
                print(f"Error processing {path_arg}: {str(e)}", file=sys.stderr)
                raise
        else:
            print(
                f"Warning: {path_arg} is not a valid file or directory", file=sys.stderr
            )

    if not layout_data:
        print("Error: No valid keymap files found", file=sys.stderr)
        return 1

    # Generate the header file and print to stdout
    header = generate_keymap_header(layout_data)
    print(header)

    return 0


if __name__ == "__main__":
    main()
