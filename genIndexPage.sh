#!/bin/sh
OUT="src/indexPage.h"
TEST_INDEX="index.html"
SOURCE_HTML="portal/index.html"
SOURCE_JS="portal/index.js"

cat ${SOURCE_JS} | python -m rjsmin > /tmp/jsmin
# NOTE: Without minification, just
# cat ${SOURCE_JS} > /tmp/jsmin

sed -f - "$SOURCE_HTML" > ${TEST_INDEX} << EOF
/<!-- CODE HERE -->/ {
 r /tmp/jsmin
 d
}
s/^ *//g
EOF

# Compress the HTML with gzip
gzip --best -c ${TEST_INDEX} > /tmp/index_html.gz

cat <<EOF > $OUT
#ifndef _INDEXPAGE_H
#define _INDEXPAGE_H
#include <stdint.h>
#include <stddef.h>
const size_t index_html_gz_len = $(stat -c%s /tmp/index_html.gz);
const uint8_t index_html[$(stat -c%s /tmp/index_html.gz)] PROGMEM = {
EOF

hexdump -v -e '16/1 "0x%02x, " "\n"' /tmp/index_html.gz | sed -e 's/0x  ,//g' -e 's/, $//' >> $OUT
# cat ${TEST_INDEX} >> $OUT
cat <<EOF >> $OUT
};
#endif
EOF
