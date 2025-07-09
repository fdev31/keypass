#!/bin/bash

# This script regenerates all the PNG icon files from the SVG logo.

# Exit immediately if a command exits with a non-zero status.
set -e

# Define the source SVG file.
SVG_FILE="android/icon.svg"

# Define the output base path.
OUTPUT_BASE_PATH="android/app/src/main/res"

# Create the necessary mipmap directories.
mkdir -p ${OUTPUT_BASE_PATH}/mipmap-mdpi
mkdir -p ${OUTPUT_BASE_PATH}/mipmap-hdpi
mkdir -p ${OUTPUT_BASE_PATH}/mipmap-xhdpi
mkdir -p ${OUTPUT_BASE_PATH}/mipmap-xxhdpi
mkdir -p ${OUTPUT_BASE_PATH}/mipmap-xxxhdpi

# Generate the standard launcher icons.
magick $SVG_FILE -resize 48x48 ${OUTPUT_BASE_PATH}/mipmap-mdpi/ic_launcher.png
magick $SVG_FILE -resize 72x72 ${OUTPUT_BASE_PATH}/mipmap-hdpi/ic_launcher.png
magick $SVG_FILE -resize 96x96 ${OUTPUT_BASE_PATH}/mipmap-xhdpi/ic_launcher.png
magick $SVG_FILE -resize 144x144 ${OUTPUT_BASE_PATH}/mipmap-xxhdpi/ic_launcher.png
magick $SVG_FILE -resize 192x192 ${OUTPUT_BASE_PATH}/mipmap-xxxhdpi/ic_launcher.png

# Generate the round launcher icons.
magick $SVG_FILE -resize 48x48 -alpha on -background none -vignette 0 ${OUTPUT_BASE_PATH}/mipmap-mdpi/ic_launcher_round.png
magick $SVG_FILE -resize 72x72 -alpha on -background none -vignette 0 ${OUTPUT_BASE_PATH}/mipmap-hdpi/ic_launcher_round.png
magick $SVG_FILE -resize 96x96 -alpha on -background none -vignette 0 ${OUTPUT_BASE_PATH}/mipmap-xhdpi/ic_launcher_round.png
magick $SVG_FILE -resize 144x144 -alpha on -background none -vignette 0 ${OUTPUT_BASE_PATH}/mipmap-xxhdpi/ic_launcher_round.png
magick $SVG_FILE -resize 192x192 -alpha on -background none -vignette 0 ${OUTPUT_BASE_PATH}/mipmap-xxxhdpi/ic_launcher_round.png

echo "Icons regenerated successfully."
