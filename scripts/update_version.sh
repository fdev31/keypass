#!/bin/sh
# chdir to the script folder
cd "$(dirname "$0")"
# Get the latest tag from git
tag=$(git describe --tags --abbrev=0)
cat  > ../src/version.h <<EOF
#ifndef _VERSION_NUMBER
#define _VERSION_NUMBER
#define VERSION "$tag"
#endif
EOF
