#!/bin/sh
OUT="src/indexPage.h"
cat <<EOF > $OUT
#ifndef _INDEXPAGE_H
#define _INDEXPAGE_H
const char index_html[] PROGMEM = R"=====(
EOF
cat index.html >> $OUT
cat <<EOF >> $OUT
)=====";
#endif
EOF
