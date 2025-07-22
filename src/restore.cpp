#include "restore.h"
#include "constants.h"
#include "importexport.h"
#include "streamadapter.h"
#include <Arduino.h>

void restoreSinglePassword(const String &passBlock, PasswordCallback callback,
                           int slot) {

  uint8_t *nonce;
  char name[MAX_NAME_LEN];
  char *passwordData[MAX_PASS_LEN];
  int layout;
  const char *rawBlock = passBlock.c_str();
  nonce = ((uint8_t *)rawBlock) + 1;

  parseSinglePassword(rawBlock, name, (char *)passwordData, &layout);

  if (callback) {
    callback(name, (const char *)passwordData, layout, slot, nonce);
  }
}

int restorePasswords(const String &data, PasswordCallback callback,
                     bool header) {
  int slot = 0;

  bool insideKpDump = !header;

  // Process each line from the data
  size_t pos = 0;
  while (slot < MAX_PASSWORDS && pos < data.length()) {
    size_t lineEnd = data.indexOf('\n', pos);

    // Skip invalid lines
    if (lineEnd - pos <= 1) {
      pos = lineEnd + 1; // Move past the newline
      continue;
    }

    String passBlock = data.substring(pos, lineEnd);

    // Check for KPDUMP markers
    if (header) {
      if (passBlock == DUMP_START) {
        insideKpDump = true;
        pos = lineEnd + 1; // Move past the marker
        continue;
      } else if (passBlock == DUMP_END) {
        insideKpDump = false;
        break; // End of dump data
      }
    }

    // Only process lines if we're inside a KPDUMP
    if (insideKpDump) {
      restoreSinglePassword(passBlock, callback, slot);
      slot++;
    }

    pos = lineEnd + 1; // Move past the newline
  }

  return slot;
}
