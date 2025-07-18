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
#define CHUNK_SIZE 512 // Size of each chunk
#define MAX_CHUNKS 20  // Maximum number of chunks (adjust based on your needs)
#define CHUNK_HEADER_SIZE 128 // Size of chunk header information

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
  if (NuPacket.isConnected()) {
    size_t packetSize;
    const uint8_t *data = NuPacket.read(packetSize);

    if (data && packetSize > 0) {
      // Create a null-terminated string from the received data
      char *command = (char *)malloc(packetSize + 1);
      if (command) {
        memcpy(command, data, packetSize);
        command[packetSize] = '\0';

        // Buffer for response
        uint8_t responseBuffer[CHUNK_SIZE]; // Use smaller buffer size
        size_t responseSize = 0;

        // Process the command and generate a response
        processCommand(command, responseBuffer, responseSize);

        // Send the response if there is one and we haven't already sent it via
        // chunking
        if (responseSize > 0) {
          NuPacket.write(responseBuffer, responseSize);
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
void sendChunkedResponse(const char *data, size_t dataLength,
                         uint8_t *responseBuffer, size_t &responseSize) {
  // Configuration
  const size_t MAX_CHUNK_DATA = 200; // Chunk size
  const size_t MAX_TOTAL_SIZE = MAX_CHUNKS * MAX_CHUNK_DATA;

  // Ensure we don't exceed maximum size
  if (dataLength > MAX_TOTAL_SIZE) {
    dataLength = MAX_TOTAL_SIZE;
  }

  // Calculate number of chunks needed
  int totalChunks = (dataLength + MAX_CHUNK_DATA - 1) / MAX_CHUNK_DATA;

  char header[256];
  snprintf(header, sizeof(header), "%d,%d,%d\n", dataLength, totalChunks,
           MAX_CHUNK_DATA);

  NuPacket.write((uint8_t *)header, strlen(header));

  // Send each chunk as raw data without markers
  size_t offset = 0;
  for (int i = 1; i <= totalChunks; i++) {
    // Calculate this chunk's size
    size_t chunkSize = ((dataLength - offset) < MAX_CHUNK_DATA)
                           ? (dataLength - offset)
                           : MAX_CHUNK_DATA;

    // Send the raw data chunk
    NuPacket.write((uint8_t *)(data + offset), chunkSize);

    // Update offset for next chunk
    offset += chunkSize;
  }

  // No end marker needed - the header tells the client exactly how much data to
  // expect

  // Set responseSize to 0 since we handled sending
  responseSize = 0;
}

// API Handler implementations
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

bool handleListCommand(JsonDocument &doc, uint8_t *responseBuffer,
                       size_t &responseSize) {
  // Get the password list
  String json = listPasswords();

  // Always use chunking for consistency
  sendChunkedResponse(json.c_str(), json.length(), responseBuffer,
                      responseSize);
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

bool handlePassDumpCommand(JsonDocument &doc, uint8_t *responseBuffer,
                           size_t &responseSize) {
  // Create a temporary String to hold the JSON data
  String jsonData;

  // Use a StringStreamAdapter to write to the String
  StringStreamAdapter stream;
  dumpPasswords(&stream);
  jsonData = stream.toString();

  // Send the complete JSON using chunking
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
