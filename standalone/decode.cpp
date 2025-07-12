#include "constants.h"
#include "crypto.h"
#include "importexport.h"
#include "utils.h"
#include <ChaCha.h>
#include <iostream>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#define MAX_LINE_LENGTH 1024

#include <fstream>
#include <sstream>
#include <string>

typedef void (*PasswordCallback)(const char *name, const uint8_t *passwordData,
                                 char layout, unsigned char version, int slot);

void printPasswordInfo(const char *name, const uint8_t *passwordData,
                       char layout, unsigned char version, int slot) {
  printf("Format: %d\n", (int)version);
  printf("Layout: %d\n", (int)layout);
  printf("Name: %s\n", name);
  printf("Password: %s\n", passwordData);
  for (int i = 0; i < MAX_PASS_LEN; i++) {
    printf("%02x ", passwordData[i]);
  }
  printf("\n");
}

int restorePasswords(const std::string &data, PasswordCallback callback) {
  int slot = 0;

  uint8_t passwordData[MAX_PASS_LEN];
  char name[MAX_NAME_LEN];
  unsigned char version;
  char layout;

  bool insideKpDump = false;

  // Process each line from the data
  size_t pos = 0;
  while (slot < MAX_PASSWORDS && pos < data.length()) {
    size_t lineEnd = data.find('\n', pos);
    printf("************************\n");

    // if (lineEnd == std::string::npos) {
    //   lineEnd = data.length();
    // }

    // Skip invalid lines
    if (lineEnd - pos <= 1) {
      pos = lineEnd + 1; // Move past the newline
      continue;
    }

    std::string passBlock = data.substr(pos, lineEnd - pos);

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

int main(int argc, char *argv[]) {
  FILE *inputFile = stdin;
  bool closeFile = false;

  // If a filename is provided, it's the key
  if (argc > 1) {
    if (!setPassPhrase(argv[1])) {
      fprintf(stderr, "Couldn't set the passphrase.\n");
      exit(-1);
    }
  } else {
    fprintf(stderr, "Please provide the passphrase in parameters!\n");
    exit(-1);
  }

  char line[MAX_LINE_LENGTH];
  uint8_t binData[MAX_PASS_LEN];
  uint8_t metaData[MAX_NAME_LEN + 3];
  char name[MAX_NAME_LEN];
  bool insideKpDump = false;
  int slot = 0;
  const int headerBytes = 3;

  printf("# Processing passwords...\n");

  std::string fileContents;
  if (inputFile == stdin) {
    // Read from stdin line by line
    while (fgets(line, MAX_LINE_LENGTH, inputFile)) {
      fileContents += line;
    }
  } else {
    // Read the entire file using your existing function (for non-stdin cases)
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), inputFile)) {
      fileContents += buffer;
    }
  }
  restorePasswords(fileContents, printPasswordInfo);

  if (closeFile) {
    fclose(inputFile);
  }
  return 0;
}
