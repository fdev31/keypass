#!/bin/sh
OUT="src/indexPage.h"
TEST_INDEX="index.html"
SOURCE_HTML="portal/index.html"
SOURCE_JS="portal/index.js"
sed -f - "$SOURCE_HTML" > ${TEST_INDEX} << EOF
/<!-- CODE HERE -->/ {
 r ${SOURCE_JS}
 d
}
s/^ *//g
EOF
cat <<EOF > $OUT
#ifndef _INDEXPAGE_H
#define _INDEXPAGE_H
const static char index_html[] PROGMEM = R"=====(
EOF
cat ${TEST_INDEX} >> $OUT
cat <<EOF >> $OUT
)=====";
#endif
EOF
