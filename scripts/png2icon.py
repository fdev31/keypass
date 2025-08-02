#!/usr/bin/env python3
import sys
from PIL import Image
import os
import argparse
import glob


def process_images(input_files, output_file=None, threshold=128, prefix=None):
    all_c_code = []
    all_bitmaps = []

    # Header and includes
    all_c_code.append("// Generated bitmap icons")
    all_c_code.append("#include <stdint.h>")
    all_c_code.append("")

    # Define the bitmap struct
    all_c_code.append("#ifndef BITMAP_H")
    all_c_code.append("#define BITMAP_H")
    all_c_code.append("typedef struct {")
    all_c_code.append("    const uint8_t *data;")
    all_c_code.append("    int width;")
    all_c_code.append("    int height;")
    all_c_code.append("} Bitmap;")
    all_c_code.append("")

    # Process each image
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
        all_c_code.append(
            f"// Bitmap data for {os.path.basename(input_file)} ({width}x{height})"
        )
        all_c_code.append(f"static const uint8_t {var_name}_data[] = {{")

        # Format the bitmap data in rows of 12 bytes
        for i in range(0, len(bitmap_data), 12):
            row_data = bitmap_data[i : i + 12]
            hex_values = ", ".join(f"0x{b:02X}" for b in row_data)
            all_c_code.append(f"    {hex_values},")

        all_c_code.append("};")

        # Add the bitmap struct
        all_c_code.append(f"static const Bitmap {var_name} = {{")
        all_c_code.append(f"    .data = {var_name}_data,")
        all_c_code.append(f"    .width = {width},")
        all_c_code.append(f"    .height = {height}")
        all_c_code.append("};")
        all_c_code.append("")

        # Track bitmap names for the collection array
        all_bitmaps.append(var_name)

    # Create an array of all bitmaps
    if all_bitmaps:
        collection_name = prefix + "_icons" if prefix else "icons"
        all_c_code.append("// Collection of all bitmap icons")
        all_c_code.append(f"static const Bitmap* {collection_name}[] = {{")
        for bitmap in all_bitmaps:
            all_c_code.append(f"    &{bitmap},")
        all_c_code.append("};")
        all_c_code.append("")
        all_c_code.append(
            f"static const int {collection_name}_count = {len(all_bitmaps)};"
        )
        all_c_code.append("#endif")
    all_c_code.append("")

    # Join all lines with newlines
    full_code = "\n".join(all_c_code)

    # Write to output file or print to stdout
    if output_file:
        with open(output_file, "w") as f:
            f.write(full_code)
        print(
            f"Generated bitmap data for {len(all_bitmaps)} icons, written to {output_file}"
        )
    else:
        print(full_code)

    return full_code


def main():
    parser = argparse.ArgumentParser(
        description="Convert multiple PNG images to 1-bit C bitmaps in a single file"
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
    parser.add_argument("--output", help="Output C file (defaults to stdout)")
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

    process_images(expanded_files, args.output, args.threshold, args.prefix)


if __name__ == "__main__":
    main()
