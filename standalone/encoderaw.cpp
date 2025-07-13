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

char *generateRandomString(int length) {
  // generate a string of "length" containing A-Za-z0-9
  static char buffer[100];
  if (length > 99)
    length = 99;
  for (int i = 0; i < length; i++) {
    int randomChar = rand() % 62; // 26 + 26 + 10
    if (randomChar < 26) {
      buffer[i] = 'A' + randomChar; // Uppercase letters
    } else if (randomChar < 52) {
      buffer[i] = 'a' + (randomChar - 26); // Lowercase letters
    } else {
      buffer[i] = '0' + (randomChar - 52); // Digits
    }
  }
  buffer[length + 1] = 0;
  return buffer;
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

  String line =
      dumpSinglePassword(name, passBuffer, strlen(argv[5]), layout, 1, slot);
  printf("%s\n", line.c_str());

  if (closeFile) {
    fclose(inputFile);
  }
  return 0;
}
