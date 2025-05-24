#!/bin/sh
sed -f - portal/index.html > index.html << EOF
/<!-- CODE HERE -->/ {
 r portal/index.js
 d
}
s/^ *//g
EOF
OUT="src/indexPage.h"
cat <<EOF > $OUT
#ifndef _INDEXPAGE_H
#define _INDEXPAGE_H
const static char index_html[] PROGMEM = R"=====(
EOF
cat index.html >> $OUT
cat <<EOF >> $OUT
)=====";
#endif
EOF
