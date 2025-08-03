#!/usr/bin/env python
import sys
from PIL import Image
import os
import argparse
import glob


def generate_header_content(input_files, threshold=128, prefix=None, output=None):
    header_lines = []
    all_bitmaps = []

    pfx = output.upper()

    # Header and includes
    header_lines.append("// Generated bitmap header")
    header_lines.append(f"#ifndef {pfx}_H")
    header_lines.append(f"#define {pfx}_H")
    header_lines.append("#include <stdint.h>")
    header_lines.append("")

    # Define the bitmap struct
    header_lines.append("typedef struct {")
    header_lines.append("    const uint8_t *data;")
    header_lines.append("    int width;")
    header_lines.append("    int height;")
    header_lines.append("} Bitmap;")
    header_lines.append("")

    # Process each image to generate declarations
    for input_file in input_files:
        try:
            img = Image.open(input_file)
        except Exception as e:
            print(f"Error opening image {input_file}: {e}")
            continue

        # Convert to grayscale if it isn't already
        if img.mode != "L":
            img = img.convert("L")
        width, height = img.size

        # Determine variable name from filename
        var_name = os.path.splitext(os.path.basename(input_file))[0]
        var_name = "".join(c if c.isalnum() else "_" for c in var_name)
        if var_name[0].isdigit():
            var_name = "icon_" + var_name

        # Apply prefix if provided
        if prefix:
            var_name = f"{prefix}_{var_name}"

        # Add declarations for this bitmap
        header_lines.append(
            f"// Bitmap for {os.path.basename(input_file)} ({width}x{height})"
        )
        header_lines.append(f"extern const uint8_t {var_name}_data[];")
        header_lines.append(f"extern const Bitmap {var_name};")
        header_lines.append("")

        # Track bitmap names for the collection array
        all_bitmaps.append(var_name)

    # Create declarations for the collection array
    if all_bitmaps:
        collection_name = prefix if prefix else "icons"
        header_lines.append("// Collection of all bitmap icons")
        header_lines.append(f"extern const Bitmap* {collection_name}[];")
        header_lines.append(f"const int {collection_name}_count = {len(all_bitmaps)};")
        header_lines.append("")

    header_lines.append(f"#endif // {pfx}_H")
    return "\n".join(header_lines), all_bitmaps


def generate_cpp_content(
    input_files, all_bitmaps, threshold=128, prefix=None, output=None
):
    cpp_lines = []

    # Header and includes
    cpp_lines.append("// Generated bitmap icons implementation")
    cpp_lines.append(f'#include "{output}.h"')
    cpp_lines.append("")

    # Process each image to generate definitions
    for i, input_file in enumerate(input_files):
        try:
            img = Image.open(input_file)
        except Exception as e:
            print(f"Error opening image {input_file}: {e}")
            continue

        # Convert to grayscale if it isn't already
        if img.mode != "L":
            img = img.convert("L")
        width, height = img.size

        # Determine variable name from filename
        var_name = os.path.splitext(os.path.basename(input_file))[0]
        var_name = "".join(c if c.isalnum() else "_" for c in var_name)
        if var_name[0].isdigit():
            var_name = "icon_" + var_name

        # Apply prefix if provided
        if prefix:
            var_name = f"{prefix}_{var_name}"

        # Calculate bytes needed for the bitmap
        bytes_per_row = (width + 7) // 8
        total_bytes = bytes_per_row * height

        # Process the pixel data
        pixels = list(img.getdata())
        bitmap_data = []
        for y in range(height):
            for byte_idx in range(bytes_per_row):
                byte_val = 0
                for bit_idx in range(8):
                    x = byte_idx * 8 + bit_idx
                    if x < width:
                        pixel_idx = y * width + x
                        # Set bit if pixel value is less than threshold (dark pixel)
                        if pixels[pixel_idx] < threshold:
                            byte_val |= 1 << bit_idx
                bitmap_data.append(byte_val)

        # Add this bitmap to the output
        cpp_lines.append(
            f"// Bitmap data for {os.path.basename(input_file)} ({width}x{height})"
        )
        cpp_lines.append(f"const uint8_t {var_name}_data[] = {{")
        # Format the bitmap data in rows of 12 bytes
        for i in range(0, len(bitmap_data), 12):
            row_data = bitmap_data[i : i + 12]
            hex_values = ", ".join(f"0x{b:02X}" for b in row_data)
            cpp_lines.append(f"    {hex_values},")
        cpp_lines.append("};")

        # Add the bitmap struct
        cpp_lines.append(f"const Bitmap {var_name} = {{")
        cpp_lines.append(f"    .data = {var_name}_data,")
        cpp_lines.append(f"    .width = {width},")
        cpp_lines.append(f"    .height = {height}")
        cpp_lines.append("};")
        cpp_lines.append("")

    # Create definitions for the collection array
    if all_bitmaps:
        collection_name = prefix if prefix else "icons"
        cpp_lines.append("// Collection of all bitmap icons")
        cpp_lines.append(f"const Bitmap* {collection_name}[] = {{")
        for bitmap in all_bitmaps:
            cpp_lines.append(f"    &{bitmap},")
        cpp_lines.append("};")
        cpp_lines.append("")

    return "\n".join(cpp_lines)


def process_images(
    input_files, output_base=None, threshold=128, prefix=None, output=None
):
    # Generate header content
    header_content, all_bitmaps = generate_header_content(
        input_files, threshold, prefix, output
    )

    # Generate C++ content
    cpp_content = generate_cpp_content(
        input_files, all_bitmaps, threshold, prefix, output
    )

    # Write to output files or print to stdout
    if output_base:
        header_file = output_base + ".h"
        cpp_file = output_base + ".cpp"

        with open(header_file, "w") as f:
            f.write(header_content)

        with open(cpp_file, "w") as f:
            f.write(cpp_content)

        print(
            f"Generated bitmap data for {len(all_bitmaps)} icons, written to {header_file} and {cpp_file}"
        )
    else:
        # If no output base is specified, print both to stdout with separators
        print("// ===== HEADER FILE =====")
        print(header_content)
        print("\n// ===== IMPLEMENTATION FILE =====")
        print(cpp_content)

    return header_content, cpp_content


def main():
    parser = argparse.ArgumentParser(
        description="Convert multiple PNG images to 1-bit C bitmaps in separate .h and .cpp files"
    )
    parser.add_argument(
        "input_files", nargs="+", help="Input PNG files or wildcard patterns"
    )
    parser.add_argument(
        "--threshold",
        type=int,
        default=128,
        help="Threshold value for converting to 1-bit (0-255)",
    )
    parser.add_argument(
        "-o", "--output", help="Output base filename (without extension)"
    )
    parser.add_argument("--prefix", help="Prefix for all variable names")
    args = parser.parse_args()

    # Expand any wildcards in the input files
    expanded_files = []
    for pattern in args.input_files:
        matches = glob.glob(pattern)
        if matches:
            expanded_files.extend(matches)
        else:
            print(f"Warning: No files match pattern '{pattern}'")

    if not expanded_files:
        print("Error: No input files to process")
        sys.exit(1)

    # if args.prefix is None:
    #     args.prefix = args.output.split("/")[-1]

    process_images(
        expanded_files,
        args.output,
        args.threshold,
        args.prefix,
        args.output.split("/")[-1],
    )


if __name__ == "__main__":
    main()
