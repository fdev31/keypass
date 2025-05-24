#!/bin/sh
sed -e '/<!-- CODE HERE -->/{ r portal/index.js' -e 'd; }' portal/index.html -e 's/^ *//g' > index.html
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
