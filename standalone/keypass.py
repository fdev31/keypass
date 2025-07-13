#!/bin/env python

import ctypes
import sys
from itertools import count
import argparse

# Load the shared library
try:
    # Try to load with different extensions based on platform
    for lib_name in ["./libkeypass.so", "./libkeypass.dylib", "./libkeypass.dll"]:
        try:
            lib = ctypes.CDLL(lib_name)
            break
        except OSError:
            continue
    else:
        raise RuntimeError(
            "Could not load the keypass library. Make sure it exists in the current directory."
        )
except Exception as e:
    raise RuntimeError(f"Error loading library: {e}")

# Define function signatures

# c_dump_single_password function
lib.c_dump_single_password.argtypes = [
    ctypes.c_char_p,  # label
    ctypes.c_char_p,  # password
    ctypes.c_char,  # layout
    ctypes.c_ubyte,  # version
    ctypes.c_int,  # index
]
lib.c_dump_single_password.restype = ctypes.c_char_p

# c_parse_single_password function
lib.c_parse_single_password.argtypes = [
    ctypes.c_char_p,  # rawdata
    ctypes.c_char_p,  # label
    ctypes.c_char_p,  # password
    ctypes.POINTER(ctypes.c_char),  # layout
    ctypes.POINTER(ctypes.c_ubyte),  # version
    ctypes.c_int,  # index
]
lib.c_parse_single_password.restype = ctypes.c_bool

# c_set_passphrase function
lib.c_set_passphrase.argtypes = [ctypes.c_char_p]  # phrase
lib.c_set_passphrase.restype = None


# Python wrapper functions
def dump_single_password(
    label: str, password: str, layout: int, version: int, index: int
) -> bytes:
    """
    Wrapper for c_dump_single_password function.

    Args:
        label: The password label
        password: The password string
        layout: Single character layout identifier
        version: The version byte
        index: The index value

    Returns:
        A bytes object with the encrypted/formatted password data
    """

    result = lib.c_dump_single_password(
        label,
        password,
        ctypes.c_char(layout),
        ctypes.c_ubyte(version),
        index,
    )

    if result:
        # Get the size of the data, assuming it's null-terminated
        # or you have a function to get the size.
        # Here, we'll find the length of the c-string.
        return result
    return None


def parse_single_password(rawdata: str, index: int) -> dict[str, any]:
    """
    Wrapper for c_parse_single_password function.
    """

    # Allocate buffers for output parameters
    password_buffer = ctypes.create_string_buffer(40)
    label_buffer = ctypes.create_string_buffer(40)
    # password_size = ctypes.c_int(0)
    layout = ctypes.c_char(0)
    version = ctypes.c_ubyte(0)

    # Make the call
    success = lib.c_parse_single_password(
        ctypes.c_char_p(rawdata.encode("utf-8")),
        label_buffer,
        password_buffer,
        ctypes.byref(layout),
        ctypes.byref(version),
        ctypes.c_int(index),
    )

    if success:
        # Try a different approach to access buffer contents
        label_str = label_buffer.value.decode("utf-8")
        password_str = password_buffer.value.decode("utf-8")

        return {
            "label": label_str,
            "password": password_str,
            "layout": ord(layout.value),
            "version": version.value,
        }

    return None


def set_passphrase(phrase: str) -> None:
    """
    Wrapper for c_set_passphrase function.
    """
    phrase_bytes = phrase.encode("utf-8")
    lib.c_set_passphrase(ctypes.c_char_p(phrase_bytes))


def main():
    parser = argparse.ArgumentParser(description="Manage keypass codes")
    parser.add_argument("--phrase", "-c", required=True, help="Passphrase to use")
    subparsers = parser.add_subparsers(dest="command", help="Command to execute")

    # Setup dump command
    dump_parser = subparsers.add_parser(
        "dump", help="Generate a code from password data"
    )
    dump_parser.add_argument(
        "--label", "-l", required=True, help="Label for the password"
    )
    dump_parser.add_argument(
        "--password", "-p", required=True, help="Password to encode"
    )
    dump_parser.add_argument(
        "--layout", "-y", type=int, default=0, help="Keyboard layout (default: 0)"
    )
    dump_parser.add_argument(
        "--version", "-v", type=int, default=1, help="Version (default: 1)"
    )
    dump_parser.add_argument(
        "--index", "-i", type=int, default=0, help="Index (default: 0)"
    )

    # Setup parse command
    parse_parser = subparsers.add_parser("parse", help="Parse codes from stdin")
    parse_parser.add_argument(
        "--index",
        "-i",
        type=int,
        default=0,
        help="Entry index for parsing (default: 0)",
    )
    parse_parser.add_argument(
        "--inline", type=str, default="", help="Inline code (instead of stdin)"
    )

    args = parser.parse_args()
    set_passphrase(args.phrase)

    if args.command == "dump":
        code = dump_single_password(
            args.label.encode("utf-8"),
            args.password.encode("utf-8"),
            args.layout,
            args.version,
            args.index,
        )
        if code:
            sys.stdout.buffer.write(code)
            print("")
    elif args.command == "parse":

        def printResult(result):
            print(result)

        if args.inline:
            result = parse_single_password(args.inline, args.index)
            printResult(result)
        else:
            counter = count()
            num = next(counter)
            for line in sys.stdin:
                code = line.strip()
                if code and code[0] != "#":
                    try:
                        result = parse_single_password(code, args.index + num)
                        printResult(result)
                    except Exception as e:
                        print(f"Error parsing code {code}: {e}")
                    else:
                        num = next(counter)
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
