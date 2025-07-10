#include "ChaCha.h"
#include "crypto.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#define MAX_PASS_LEN 32
#define MAX_NAME_LEN 30
#define MAX_LINE_LENGTH 1024
#define MAX_PASSWORDS 100
#define CRYPTO_OVERHEAD 16

// External functions declaration
extern std::string hexDump(const char *hexStr, size_t maxLen);
extern void hexParse(const char *hexStr, uint8_t *binData, size_t maxLen);
extern bool setPassPhrase(const char *passphrase);
extern void encryptPassword(const char *password, uint8_t *result, int index);
extern void decryptPassword(const uint8_t *encrypted, char *result, int index);
ChaCha chacha(20); // ChaCha with 20 rounds (ChaCha20)
                   //
// Helper function to create entry name
const char *mkEntryName(int num) {
  static char name[16];
  snprintf(name, sizeof(name), "p%d", num);
  return name;
}
//
std::string testDumpPassword(char *password) {
  char buffer[MAX_PASS_LEN + CRYPTO_OVERHEAD];
  char name[MAX_NAME_LEN + CRYPTO_OVERHEAD];
  char layout = 0;
  char version = 0;
  std::string result;
  const char *key = mkEntryName(0);
  *buffer = 0;
  randomizeBuffer((uint8_t *)name, MAX_NAME_LEN);
  result += hexDump(&version, 1);
  result += hexDump(&layout, 1);
  result += hexDump(name, MAX_NAME_LEN + CRYPTO_OVERHEAD);
  result += " ";
  result += hexDump(buffer, MAX_PASS_LEN + CRYPTO_OVERHEAD);
  result += "\n";
  encryptPassword(password, (uint8_t *)name, 0);
  return result;
}
//
void testEncryptDecrypt() {
  // Test data
  const char *original = "XXXthis is a nie long passwordXX";
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
  size_t plaintext_len = strlen(original) + 1;
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
  chacha.encrypt(ciphertext, (const uint8_t *)test, strlen(test) + 1);

  // Print encrypted data
  printf("Test Encrypted: ");
  for (int i = 0; i < strlen(test) + 1; i++) {
    printf("%02x ", ciphertext[i]);
  }
  printf("\n");

  // Reset ChaCha with same key and nonce
  chacha.setKey(key, 32);
  chacha.setIV(nonce, 12);

  // Decrypt
  chacha.decrypt(decrypted, ciphertext, strlen(test) + 1);

  // Print decrypted data
  printf("Test Decrypted: ");
  for (int i = 0; i < strlen(test) + 1; i++) {
    printf("%02x ", decrypted[i]);
  }
  printf("\n");

  printf("Test String: %s\n", decrypted);
}

void printHex(const char *label, const uint8_t *data, size_t len) {
  printf("%s: ", label);
  for (size_t i = 0; i < len; i++) {
    printf("%02x ", data[i]);
  }
  printf("\n");
}

int main(int argc, char *argv[]) {

  testEncryptDecrypt(); // Test the encrypt/decrypt functionality
  testChaCha();         // Test the ChaCha implementation
  const uint8_t password[MAX_PASS_LEN] = "this is clear!";
  uint8_t result[MAX_PASS_LEN];
  uint8_t clear[MAX_PASS_LEN];

  encryptPassword((const char *)password, result, 1);
  decryptPassword(result, (char *)clear, 1);

  int plaintext_len = strlen((char *)password);
  printHex("Original", password, plaintext_len); // Include null terminator
  printHex("Encrypted", result,
           16 + plaintext_len + 16); // nonce + length + ciphertext + tag
  printHex("Decrypted", clear, plaintext_len);

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

  // Read the input line by line
  while (slot < MAX_PASSWORDS && fgets(line, sizeof(line), inputFile)) {
    // Remove newline character if present
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
      len--;
    }

    // Skip empty lines
    if (len == 0) {
      continue;
    }

    // Check for KPDUMP markers
    if (strcmp(line, "#KPDUMP") == 0) {
      insideKpDump = true;
      printf("# Found KPDUMP header\n");
      continue;
    } else if (strcmp(line, "#/KPDUMP") == 0) {
      insideKpDump = false;
      printf("# Found KPDUMP footer\n");
      break;
    }

    // Only process lines if we're inside a KPDUMP section
    if (insideKpDump) {
      char *spacePos = strchr(line, ' ');
      if (!spacePos) {
        fprintf(stderr,
                "Warning: Invalid line format (missing space separator): %s\n",
                line);
        continue;
      }

      // Null-terminate the metaBlock part
      *spacePos = '\0';

      const char *metaBlock = line;
      const char *passBlock = spacePos + 1;

      // Convert hex strings back to binary data
      memset(metaData, 0, sizeof(metaData));
      memset(binData, 0, sizeof(binData));

      hexParse(metaBlock, metaData, MAX_NAME_LEN + headerBytes);
      hexParse(passBlock, binData, MAX_PASS_LEN);

      // Extract metadata
      int format = metaData[0];
      int layout = metaData[1];

      // Decrypt name if needed
      decryptPassword(metaData + headerBytes - 1, name, slot);

      printf("Password %d:\n", slot);
      printf("  Name: %s\n", name);
      printf("  Format: %d\n", format);
      printf("  Layout: %d\n", layout);

      // Decrypt password
      char passwordText[MAX_PASS_LEN];
      memset(passwordText, 0, sizeof(passwordText));
      decryptPassword(binData, passwordText, slot);
      printf("  Password: %s\n", passwordText);

      // Test re-encryption
      uint8_t reencrypted[MAX_PASS_LEN];
      encryptPassword(passwordText, reencrypted, slot);

      // Verify re-encryption matches original
      bool matches = (memcmp(reencrypted, binData, MAX_PASS_LEN) == 0);
      printf("  Re-encryption test: %s\n", matches ? "PASSED" : "FAILED");

      printf("\n");
      slot++;
    }
  }

  printf("# Processed %d passwords\n", slot);

  if (closeFile) {
    fclose(inputFile);
  }

  return 0;
}
