#include "constants.h"
#include "crypto.h"
#include "importexport.h"
#include "restore.h"
#include "utils.h"
#include <ChaCha.h>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#define MAX_LINE_LENGTH 1024

/**
 * Converts a byte array to a JSON-compatible string with proper escaping.
 *
 * @param bytes The input byte array
 * @param len Length of the byte array
 * @return Newly allocated null-terminated string (caller must free)
 *         or NULL if allocation fails
 */
char *bytes_to_json_string(const char *bytes, size_t len) {
  if (bytes == NULL) {
    return NULL;
  }

  // Worst case: each byte becomes \uXXXX (6 chars)
  size_t max_size = len * 6 + 1; // +1 for null terminator
  char *result = (char *)malloc(max_size);
  if (result == NULL) {
    return NULL;
  }

  size_t pos = 0;
  for (size_t i = 0; i < len; i++) {
    unsigned char c = bytes[i];

    // Handle special JSON escape sequences
    if (c == '"') {
      result[pos++] = '\\';
      result[pos++] = '"';
    } else if (c == '\\') {
      result[pos++] = '\\';
      result[pos++] = '\\';
    } else if (c == '\b') {
      result[pos++] = '\\';
      result[pos++] = 'b';
    } else if (c == '\f') {
      result[pos++] = '\\';
      result[pos++] = 'f';
    } else if (c == '\n') {
      result[pos++] = '\\';
      result[pos++] = 'n';
    } else if (c == '\r') {
      result[pos++] = '\\';
      result[pos++] = 'r';
    } else if (c == '\t') {
      result[pos++] = '\\';
      result[pos++] = 't';
    } else if (c < 32 || c >= 127) {
      result[pos++] = '\\';
      result[pos++] = 'x';
      // Use \u escaping for control chars and non-ASCII
      sprintf(result + pos, "%02x", c);
      pos += 2;
    } else {
      // Regular printable ASCII
      result[pos++] = c;
    }

    // Safety check to prevent buffer overflow
    if (pos >= max_size - 7) { // -7 to ensure room for next char
      result[pos] = '\0';
      return result;
    }
  }

  result[pos] = '\0';
  return result;
}

bool isFirstPrint = true;

void printPasswordInfo(const char *name, const char *passwordData, int layout,
                       int slot, uint8_t *nonce) {

  // if (getenv("KPASS")) {
  //   decryptBuffer((uint8_t *)passwordData, (char *)passwordData, slot,
  //                 passwordData + 1,
  //                 MAX_PASS_LEN);
  // }
  if (isFirstPrint) {
    printf("\n{\n");
  } else {
    printf(",\n{\n");
  }
  printf("  \"name\": \"%s\",\n", name);
  printf("  \"layout\": %d,\n", (int)layout);
  printf("  \"password\": \"%s\"",
         bytes_to_json_string(passwordData, MAX_PASS_LEN));
  printf("\n}");
  if (isFirstPrint)
    isFirstPrint = false;
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

  String fileContents;
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
  printf("[");
  restorePasswords(fileContents, printPasswordInfo, false);
  printf("\n]\n");

  if (closeFile) {
    fclose(inputFile);
  }
  return 0;
}
