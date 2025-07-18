#include "bluetooth.h"
#include "NimBLEDevice.h"
#include "NuPacket.hpp"
#include "configuration.h"
#include "constants.h"
#include "crypto.h"
#include "graphics.h"
#include "hid.h"
#include "password.h"
#include <Arduino.h>
#include <ArduinoJson.h>

#define DEVICE_NAME DEFAULT_WIFI_SSID

// JSON response buffer size
#define JSON_BUFFER_SIZE 2048
#define CHUNK_SIZE 20  // Size of each data chunk
#define MAX_CHUNKS 200 // Maximum number of chunks
#define CHUNKS_PER_ITERATION 3
static String
    persistentChunkData; // Stores the data being chunked to keep it in scope
//
// Global chunking state
struct ChunkState {
  bool inProgress;        // Whether chunking is in progress
  const char *data;       // Pointer to data being sent
  size_t dataLength;      // Total data length
  size_t offset;          // Current offset in the data
  int totalChunks;        // Total number of chunks
  int chunksSent;         // Number of chunks already sent
  int chunksPerIteration; // How many chunks to send per loop
};

static ChunkState chunkState = {.inProgress = false,
                                .data = nullptr,
                                .dataLength = 0,
                                .offset = 0,
                                .totalChunks = 0,
                                .chunksSent = 0,
                                .chunksPerIteration = CHUNKS_PER_ITERATION};

// Command parsing and response functions
void processCommand(const char *command, uint8_t *responseBuffer,
                    size_t &responseSize);
void sendResponse(int status, const char *message, uint8_t *responseBuffer,
                  size_t &responseSize);

// API handlers
bool handleTypeRawCommand(JsonDocument &doc, uint8_t *responseBuffer,
                          size_t &responseSize);
bool handleTypePassCommand(JsonDocument &doc, uint8_t *responseBuffer,
                           size_t &responseSize);
bool handleFetchPassCommand(JsonDocument &doc, uint8_t *responseBuffer,
                            size_t &responseSize);
bool handleEditPassCommand(JsonDocument &doc, uint8_t *responseBuffer,
                           size_t &responseSize);
bool handleListCommand(JsonDocument &doc, uint8_t *responseBuffer,
                       size_t &responseSize);
bool handleFactoryResetCommand(JsonDocument &doc, uint8_t *responseBuffer,
                               size_t &responseSize);
bool handleWifiPassCommand(JsonDocument &doc, uint8_t *responseBuffer,
                           size_t &responseSize);
bool handlePassPhraseCommand(JsonDocument &doc, uint8_t *responseBuffer,
                             size_t &responseSize);
bool handlePassDumpCommand(JsonDocument &doc, uint8_t *responseBuffer,
                           size_t &responseSize);
bool handleRestoreCommand(JsonDocument &doc, uint8_t *responseBuffer,
                          size_t &responseSize);

void bluetoothSetup() {
  // Initialize BLE stack
  NimBLEDevice::init(DEVICE_NAME);

  // Configure advertising
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName(DEVICE_NAME);
  pAdvertising->addServiceUUID(NORDIC_UART_SERVICE_UUID);

  // Set scan response data
  pAdvertising->setScanResponseData(pAdvertising->getAdvertisementData());

  // Set advertising intervals
  pAdvertising->setMinInterval(0x20);
  pAdvertising->setMaxInterval(0x40);

  // Start Nordic UART Service with NuPacket
  NuPacket.start();
}
void bluetoothLoop() {

  // Process incoming packets
  if (NuPacket.isConnected()) {
    // Process chunking if in progress
    if (chunkState.inProgress) {
      int chunksToSend = min(chunkState.chunksPerIteration,
                             chunkState.totalChunks - chunkState.chunksSent);

      char buffer[100];
      sprintf(buffer, "S %d %d/%d", chunksToSend, chunkState.chunksSent,
              chunkState.totalChunks);
      printText(2, buffer);

      for (int i = 0; i < chunksToSend; i++) {
        // Calculate this chunk's size
        size_t chunkSize =
            ((chunkState.dataLength - chunkState.offset) < CHUNK_SIZE)
                ? (chunkState.dataLength - chunkState.offset)
                : CHUNK_SIZE;

        // Send the raw data chunk
        NuPacket.write((uint8_t *)(chunkState.data + chunkState.offset),
                       chunkSize);

        // Update offset and count for next chunk
        chunkState.offset += chunkSize;
        chunkState.chunksSent++;

        sprintf(buffer, "C%d/%d %d", chunkState.chunksSent,
                chunkState.totalChunks, chunkSize);
        printText(2, buffer);

        if (chunkState.chunksSent >= chunkState.totalChunks) {
          chunkState.inProgress = false;
          printText(2, "TC");
          break;
        }
      }
    }

    // Handle incoming data - THIS PART WAS MISSING
    size_t packetSize;
    const uint8_t *data = NuPacket.read(packetSize);

    if (data && packetSize > 0) {
      // Create a null-terminated string from the received data
      char *command = (char *)malloc(packetSize + 1);
      if (command) {
        memcpy(command, data, packetSize);
        command[packetSize] = '\0';

        // Buffer for response
        uint8_t responseBuffer[200]; // Use smaller buffer size
        size_t responseSize = 0;
        if (chunkState.inProgress) {
          printText(2, "ERROR: Chunking already in progress");
        } else {
          // Process the command and generate a response
          processCommand(command, responseBuffer, responseSize);

          // Send the response if there is one and we haven't already sent it
          // via chunking
          if (responseSize > 0) {
            NuPacket.write(responseBuffer, responseSize);
          }
        }

        free(command);
      }
    }
  }
}

void processCommand(const char *command, uint8_t *responseBuffer,
                    size_t &responseSize) {
  // Parse JSON command
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, command);

  if (error) {
    sendResponse(400, "Invalid JSON format", responseBuffer, responseSize);
    return;
  }

  // Check if command is present
  if (!doc.containsKey("cmd")) {
    sendResponse(400, "Missing 'cmd' field", responseBuffer, responseSize);
    return;
  }

  const char *cmd = doc["cmd"];
  bool handled = false;

  // Route to appropriate handler
  if (strcmp(cmd, "typeRaw") == 0) {
    handled = handleTypeRawCommand(doc, responseBuffer, responseSize);
  } else if (strcmp(cmd, "typePass") == 0) {
    handled = handleTypePassCommand(doc, responseBuffer, responseSize);
  } else if (strcmp(cmd, "fetchPass") == 0) {
    handled = handleFetchPassCommand(doc, responseBuffer, responseSize);
  } else if (strcmp(cmd, "editPass") == 0) {
    handled = handleEditPassCommand(doc, responseBuffer, responseSize);
  } else if (strcmp(cmd, "list") == 0) {
    handled = handleListCommand(doc, responseBuffer, responseSize);
  } else if (strcmp(cmd, "reset") == 0) {
    handled = handleFactoryResetCommand(doc, responseBuffer, responseSize);
  } else if (strcmp(cmd, "updateWifiPass") == 0) {
    handled = handleWifiPassCommand(doc, responseBuffer, responseSize);
  } else if (strcmp(cmd, "passphrase") == 0) {
    handled = handlePassPhraseCommand(doc, responseBuffer, responseSize);
  } else if (strcmp(cmd, "dump") == 0) {
    handled = handlePassDumpCommand(doc, responseBuffer, responseSize);
  } else if (strcmp(cmd, "restore") == 0) {
    handled = handleRestoreCommand(doc, responseBuffer, responseSize);
  }

  if (!handled) {
    sendResponse(404, "Unknown command", responseBuffer, responseSize);
  }
}

void sendResponse(int status, const char *message, uint8_t *responseBuffer,
                  size_t &responseSize) {
  StaticJsonDocument<256> doc;
  doc["status"] = status;
  doc["message"] = message;

  // Serialize JSON to buffer
  responseSize = serializeJson(doc, (char *)responseBuffer, JSON_BUFFER_SIZE);
}
// API Handler implementations
void sendChunkedResponse(const char *data, size_t dataLength,
                         uint8_t *responseBuffer, size_t &responseSize) {
  // Check if chunking is already in progress
  if (chunkState.inProgress) {
    printText(2, "ERROR: Chunking already in progress");
    responseSize = 0;
    return;
  }

  // Ensure we don't exceed maximum size
  const size_t MAX_TOTAL_SIZE = MAX_CHUNKS * CHUNK_SIZE;
  if (dataLength > MAX_TOTAL_SIZE) {
    dataLength = MAX_TOTAL_SIZE;
  }

  // Copy the data to our persistent buffer
  persistentChunkData = String(data, dataLength);

  char buffer[100];
  sprintf(buffer, "S %d ", dataLength);
  printText(2, buffer);

  // Initialize chunking state
  chunkState.inProgress = true;
  chunkState.data = persistentChunkData.c_str(); // Point to our persistent copy
  chunkState.dataLength = dataLength;
  chunkState.offset = 0;
  chunkState.totalChunks = (dataLength + CHUNK_SIZE - 1) / CHUNK_SIZE;
  chunkState.chunksSent = 0;
  chunkState.chunksPerIteration = CHUNKS_PER_ITERATION;

  sprintf(buffer, "C %d*%d", chunkState.totalChunks, CHUNK_SIZE);
  printText(2, buffer);

  // Send header
  char header[256];
  snprintf(header, sizeof(header), "%d,%d,%d\n", dataLength,
           chunkState.totalChunks, CHUNK_SIZE);
  NuPacket.write((uint8_t *)header, strlen(header));

  printText(2, "I");

  // Set responseSize to 0 since we handled sending
  responseSize = 0;
}
bool handleTypeRawCommand(JsonDocument &doc, uint8_t *responseBuffer,
                          size_t &responseSize) {
  if (!doc.containsKey("text")) {
    sendResponse(400, "Missing 'text' parameter", responseBuffer, responseSize);
    return true;
  }

  const char *text = doc["text"];
  int layout = 0; // Default layout

  if (doc.containsKey("layout")) {
    layout = doc["layout"];
  }

  bool sendNewline = true;
  if (doc.containsKey("ret") && strcmp(doc["ret"], "false") == 0) {
    sendNewline = false;
  }

  if (typeRawText(text, layout, sendNewline)) {
    sendResponse(200, "OK", responseBuffer, responseSize);
  } else {
    sendResponse(500, "Failed to type text", responseBuffer, responseSize);
  }

  return true;
}

bool handleTypePassCommand(JsonDocument &doc, uint8_t *responseBuffer,
                           size_t &responseSize) {
  if (!doc.containsKey("id")) {
    sendResponse(400, "Missing 'id' parameter", responseBuffer, responseSize);
    return true;
  }

  int id = doc["id"];
  int layout = -1;

  if (doc.containsKey("layout")) {
    layout = doc["layout"];
  }

  bool sendNewline = true;
  if (doc.containsKey("ret") && strcmp(doc["ret"], "false") == 0) {
    sendNewline = false;
  }

  if (typePassword(id, layout, sendNewline)) {
    sendResponse(200, "Password typed successfully", responseBuffer,
                 responseSize);
  } else {
    sendResponse(400, "Invalid password ID", responseBuffer, responseSize);
  }

  return true;
}

bool handleFetchPassCommand(JsonDocument &doc, uint8_t *responseBuffer,
                            size_t &responseSize) {
  if (!doc.containsKey("id")) {
    sendResponse(400, "Missing 'id' parameter", responseBuffer, responseSize);
    return true;
  }

  int id = doc["id"];
  const char *password = fetchPassword(id);

  if (password) {
    sendResponse(200, password, responseBuffer, responseSize);
  } else {
    sendResponse(400, "Invalid password ID", responseBuffer, responseSize);
  }

  return true;
}

bool handleEditPassCommand(JsonDocument &doc, uint8_t *responseBuffer,
                           size_t &responseSize) {
  if (!doc.containsKey("id")) {
    sendResponse(400, "Missing 'id' parameter", responseBuffer, responseSize);
    return true;
  }

  int id = doc["id"];
  const char *name = nullptr;
  const char *password = nullptr;
  int layout = -2; // Special value meaning "don't change"

  if (doc.containsKey("name")) {
    name = doc["name"];
  }

  if (doc.containsKey("password")) {
    password = doc["password"];
  }

  if (doc.containsKey("layout")) {
    layout = doc["layout"];
  }

  if (editPassword(id, name, password, layout)) {
    sendResponse(200, "Password edited successfully", responseBuffer,
                 responseSize);
  } else {
    sendResponse(400, "Invalid parameter", responseBuffer, responseSize);
  }

  return true;
}

bool handleFactoryResetCommand(JsonDocument &doc, uint8_t *responseBuffer,
                               size_t &responseSize) {
  factoryReset();
  sendResponse(200, "Factory reset completed", responseBuffer, responseSize);
  return true;
}

bool handleWifiPassCommand(JsonDocument &doc, uint8_t *responseBuffer,
                           size_t &responseSize) {
  if (!doc.containsKey("newPass")) {
    sendResponse(400, "Missing 'newPass' parameter", responseBuffer,
                 responseSize);
    return true;
  }

  const char *pass = doc["newPass"];
  if (setWifiPassword(pass)) {
    sendResponse(200, "WiFi password updated", responseBuffer, responseSize);
  } else {
    sendResponse(500, "Failed to update WiFi password", responseBuffer,
                 responseSize);
  }

  return true;
}

bool handlePassPhraseCommand(JsonDocument &doc, uint8_t *responseBuffer,
                             size_t &responseSize) {
  if (!doc.containsKey("p")) {
    sendResponse(400, "Missing 'p' parameter", responseBuffer, responseSize);
    return true;
  }

  const char *phrase = doc["p"];
  if (setupPassphrase(phrase)) {
    sendResponse(200, "Passphrase set successfully", responseBuffer,
                 responseSize);
  } else {
    sendResponse(500, "Passphrase couldn't be set", responseBuffer,
                 responseSize);
  }

  return true;
}

bool handleListCommand(JsonDocument &doc, uint8_t *responseBuffer,
                       size_t &responseSize) {
  // If chunking is in progress, return an error
  if (chunkState.inProgress) {
    sendResponse(503, "Transfer already in progress", responseBuffer,
                 responseSize);
    return true;
  }

  // Get the password list
  String json = listPasswords();

  // Start chunked transfer
  sendChunkedResponse(json.c_str(), json.length(), responseBuffer,
                      responseSize);
  return true;
}

bool handlePassDumpCommand(JsonDocument &doc, uint8_t *responseBuffer,
                           size_t &responseSize) {
  // If chunking is in progress, return an error
  if (chunkState.inProgress) {
    sendResponse(503, "Transfer already in progress", responseBuffer,
                 responseSize);
    return true;
  }

  // Create a temporary String to hold the JSON data
  String jsonData;

  // Use a StringStreamAdapter to write to the String
  StringStreamAdapter stream;
  dumpPasswords(&stream);
  jsonData = stream.toString();

  // Start the chunked transfer
  sendChunkedResponse(jsonData.c_str(), jsonData.length(), responseBuffer,
                      responseSize);
  return true;
}

bool handleRestoreCommand(JsonDocument &doc, uint8_t *responseBuffer,
                          size_t &responseSize) {
  if (!doc.containsKey("data")) {
    sendResponse(400, "Missing 'data' parameter", responseBuffer, responseSize);
    return true;
  }

  String uploadData = doc["data"];
  String ret = restoreMCUPasswords(uploadData);
  sendResponse(200, ret.c_str(), responseBuffer, responseSize);
  return true;
}
