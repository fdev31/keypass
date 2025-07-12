#include "restore.h"
#include "constants.h"
#include "importexport.h"
#include <Arduino.h>

int restorePasswords(const String &data, PasswordCallback callback) {
  int slot = 0;

  uint8_t passwordData[MAX_PASS_LEN];
  char name[MAX_NAME_LEN];
  unsigned char version;
  char layout;

  bool insideKpDump = false;

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
    if (passBlock == "#KPDUMP") {
      insideKpDump = true;
      pos = lineEnd + 1; // Move past the marker
      continue;
    } else if (passBlock == "#/KPDUMP") {
      insideKpDump = false;
      break; // End of dump data
    }

    // Only process lines if we're inside a KPDUMP
    if (insideKpDump) {

      parseSinglePassword(passBlock.c_str(), name, (char *)passwordData, NULL,
                          &layout, &version, slot);

      if (callback) {
        callback(name, passwordData, layout, version, slot);
      }
      slot++;
    }

    pos = lineEnd + 1; // Move past the newline
  }

  return slot;
}
