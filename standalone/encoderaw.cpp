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
#include <time.h>

#define MAX_LINE_LENGTH 1024

#include <fstream>
#include <sstream>
#include <string>

int main(int argc, char *argv[]) {
  FILE *inputFile = stdin;
  bool closeFile = false;
  srandom(time(NULL));

  // If a filename is provided, it's the key
  if (argc > 1) {
    if (!setPassPhrase(argv[1])) {
      fprintf(stderr, "Couldn't set the passphrase.\n");
      exit(-1);
    }
  } else {
    fprintf(stderr, "Syntax: <passphrase> <slot> <layout> <name> <pass>\n");
    exit(-1);
  }

  char passBuffer[MAX_PASS_LEN];
  char name[MAX_NAME_LEN];
  int slot = atoi(argv[2]);
  int layout = atoi(argv[3]);

  randomizeBuffer((uint8_t *)name, MAX_NAME_LEN);
  memcpy(name, argv[4], strlen(argv[4]) + 1);
  randomizeBuffer((uint8_t *)passBuffer, MAX_PASS_LEN);
  memcpy(passBuffer, argv[5], strlen(argv[5]) + 1);

  if (getenv("KPASS")) {
    encryptBuffer((const char *)passBuffer, (uint8_t *)passBuffer, slot,
                  MAX_PASS_LEN);
  }

  String line = dumpSinglePassword(name, passBuffer, layout, 1, slot);
  printf("%s\n", line.c_str());

  if (closeFile) {
    fclose(inputFile);
  }
  return 0;
}
