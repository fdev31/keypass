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

ChaCha chacha(20); // ChaCha with 20 rounds (ChaCha20)
                   //

int restorePasswords2(const String &data) {
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

    // if (lineEnd == String::npos) {
    //   lineEnd = data.length();
    // }

    // Skip invalid lines
    if (lineEnd - pos <= 1) {
      pos = lineEnd + 1; // Move past the newline
      continue;
    }

    String passBlock = data.substr(pos, lineEnd - pos);

    printf("%s#\n", passBlock.c_str());
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

      // Save the binary data directly to preferences
      // const char *entryName = mkEntryName(slot);
      printf("Format: %d\n", (int)version);
      printf("Layout: %d\n", (int)layout);
      printf("Name: %s\n", name);
      // printf("Password: %s\n", passwordData);
      slot++;
    }

    pos = lineEnd + 1; // Move past the newline
  }

  return slot;
}
//
void testEncryptDecrypt() {
  // Test data
  const char *original = "XXXthis is a nice long passwordXX";
  uint8_t encrypted[100] = {0};
  char decrypted[100] = {0};

  // Create fresh ChaCha instances
  ChaCha encryptChacha(20);
  ChaCha decryptChacha(20);

  // Set the same key for both
  uint8_t key[32];
  for (int i = 0; i < 32; i++)
    key[i] = i + 1; // Simple test key
  encryptChacha.setKey(key, 32);
  decryptChacha.setKey(key, 32);

  // Set the same nonce for both
  uint8_t nonce[12];
  for (int i = 0; i < 12; i++)
    nonce[i] = i + 10; // Simple test nonce
  encryptChacha.setIV(nonce, 12);
  decryptChacha.setIV(nonce, 12);

  // Encrypt
  size_t plaintext_len = 100;
  encryptChacha.encrypt(encrypted, (const uint8_t *)original, plaintext_len);

  // Decrypt
  decryptChacha.decrypt((uint8_t *)decrypted, encrypted, plaintext_len);

  // Print results
  printf("Test original: %s\n", original);
  printf("Test decrypted: %s\n", decrypted);
  printf("Test success: %s\n",
         (strcmp(original, decrypted) == 0) ? "YES" : "NO");
}

void testChaCha() {
  const char *test = "this is clear, but also very long...!";
  uint8_t ciphertext[64] = {0};
  uint8_t decrypted[64] = {0};
  uint8_t nonce[12] = {0};
  uint8_t key[32] = {0};

  // Set a test key and nonce
  for (int i = 0; i < 32; i++)
    key[i] = i;
  for (int i = 0; i < 12; i++)
    nonce[i] = i;

  // Reset ChaCha
  chacha.setKey(key, 32);
  chacha.setIV(nonce, 12);

  // Encrypt
  chacha.encrypt(ciphertext, (const uint8_t *)test, 64);

  // Print encrypted data
  printf("\nTest Encrypted: ");
  for (int i = 0; i < 64; i++) {
    printf("%02x ", ciphertext[i]);
  }
  printf("\n");

  chacha.clear();
  // Reset ChaCha with same key and nonce
  chacha.setKey(key, 32);
  chacha.setIV(nonce, 12);

  // Decrypt
  chacha.decrypt(decrypted, ciphertext, 64);

  // Print decrypted data
  printf("Test Decrypted: %s\n", decrypted);
  for (int i = 0; i < 64 + 1; i++) {
    printf("%02x ", decrypted[i]);
  }
  printf("\n");

  printf("Test String: %s\n", decrypted);
}

void printHex(const char *label, const uint8_t *data, size_t len) {
  printf("%20s: ", label);
  for (size_t i = 0; i < len; i++) {
    printf("%02x-", data[i]);
  }
  printf("\n");
}

int main(int argc, char *argv[]) {

  testEncryptDecrypt(); // Test the encrypt/decrypt functionality
  printf(
      "#####################################################################");
  testChaCha(); // Test the ChaCha implementation

  printf("#####################################################################"
         "######\n");
  uint8_t password[MAX_PASS_LEN] = "this is clear, and fun!";
  uint8_t result[MAX_PASS_LEN];
  uint8_t clear[MAX_PASS_LEN];
  randomizeBuffer(password, 10);

  encryptBuffer((const char *)password, result, 1, MAX_PASS_LEN);
  decryptPassword(result, (char *)clear, 1, MAX_PASS_LEN);

  int plaintext_len = MAX_PASS_LEN;
  printHex("\nOriginal", password, plaintext_len); // Include null terminator
  printHex("\nEncrypted", result,
           plaintext_len); // nonce + length + ciphertext + tag
  printHex("\nDecrypted", clear, plaintext_len);

  printf("#####################################################################"
         "######\n");
  printf("%s\n",
         dumpSinglePassword("Mr label", "pikaCouille36", 14, 1, 5, 2).c_str());
  String res = dumpSinglePassword("Mr label", "pikaCouille36", 14, 1, 5, 1);
  printf("%s\n", res.c_str());
  parseSinglePassword(res.c_str(), (char *)clear, (char *)password,
                      &plaintext_len, (char *)&result[0],
                      (unsigned char *)&result[1], 1);
  printf("Parsed label: %s\n", clear);
  printf("Parsed password: %s\n", password);
  String test = String("#KPDUMP\n");
  test += res;
  test += String("\n#/KPDUMP\n");

  printf("#####################################################################"
         "######\n");

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
  restorePasswords2(fileContents);

  if (closeFile) {
    fclose(inputFile);
  }
  return 0;
}
