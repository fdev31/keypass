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
  randomizeBuffer((uint8_t *)buffer, 100);
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
  buffer[length] = 0;
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
    fprintf(stderr, "Please provide the passphrase in parameters!\n");
    exit(-1);
  }

  char name[MAX_NAME_LEN];

  printf("# Processing passwords...\n");

  /*
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
  */
  printf("#KPDUMP\n");
  for (int i = 0; i < 32; i++) {
    sprintf(name, "Password length %d", i);
    String line = dumpSinglePassword(name, generateRandomString(i),
                                     random() % 3 - 1, 1, i);
    printf("%s\n", line.c_str());
  }
  printf("#/KPDUMP\n");

  if (closeFile) {
    fclose(inputFile);
  }
  return 0;
}
