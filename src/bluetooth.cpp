#include "bluetooth.h"
#include "NimBLEDevice.h"
#include "NuPacket.hpp"
#include "WiFi.h"
#include "configuration.h"
#include "constants.h"
#include "crypto.h"
#include "graphics.h"
#include "hid.h"
#include "password.h"
#include "restore.h"
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

static bool previousBleConnected = false;

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
bool handlePassDumpOneCommand(JsonDocument &doc, uint8_t *responseBuffer,
                              size_t &responseSize);
bool handleRestoreCommand(JsonDocument &doc, uint8_t *responseBuffer,
                          size_t &responseSize);
bool handleRestoreOneCommand(JsonDocument &doc, uint8_t *responseBuffer,
                             size_t &responseSize);

#if 0
class MySecurityCallbacks : public NimBLESecurityCallbacks {
  bool onConfirmPIN(uint32_t pin) {
    char buffer[100];
    sprintf(buffer, "Confirm PIN: %06d", pin);
    printText(3, buffer);
    return true; // Accept the pairing
  }

  class MySecurityCallbacks : public NimBLESecurityCallbacks {
  public:
    uint32_t onPassKeyRequest() {
      return 123456; // Static passkey (you could generate a random one)
    }

    void onPassKeyNotify(uint32_t pass_key) {
      char buffer[100];
      sprintf(buffer, "Passkey: %06d", pass_key);
      printText(3, buffer);
    }

    bool onConfirmPIN(uint32_t pin) {
      char buffer[100];
      sprintf(buffer, "Confirm: %06d", pin);
      printText(3, buffer);
      return true; // Accept the pairing
    }

    bool onSecurityRequest() {
      return true; // Accept pairing request from client
    }

    void onAuthenticationComplete(ble_gap_conn_desc *desc) {
      if (desc->sec_state.encrypted) {
        printText(3, "Paired & Encrypted");
      } else {
        printText(3, "Pairing failed");
      }
    }
  };
}
#endif
void bluetoothSetup() {
  // Initialize BLE stack
  NimBLEDevice::init(DEVICE_NAME);

  NimBLEServer *pServer = NimBLEDevice::createServer();
  // pServer->setSecurityCallbacks(new MySecurityCallbacks());

  // Set security level
  uint8_t secLevel = BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |
                     BLE_SM_PAIR_AUTHREQ_SC;
  NimBLEDevice::setSecurityAuth(secLevel);

  // Set I/O capabilities
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO);

  // Set security initialization vector size
  NimBLEDevice::setSecurityInitKey(BLE_SM_PAIR_KEY_DIST_ENC |
                                   BLE_SM_PAIR_KEY_DIST_ID);
  NimBLEDevice::setSecurityRespKey(BLE_SM_PAIR_KEY_DIST_ENC |
                                   BLE_SM_PAIR_KEY_DIST_ID);

  // Configure advertising
  NimBLEAdvertising *pAdvertising = pServer->getAdvertising();
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
#if ENABLE_HTTP
  bool isConnected = NuPacket.isConnected();

  if (isConnected && isConnected != previousBleConnected) {
    WiFi.mode(WIFI_OFF);
    setIconStatus(ICON_WIFI, false);
  }
  previousBleConnected = isConnected;
#endif

  setIconStatus(ICON_BLUETOOTH, NuPacket.isConnected());

  // Process incoming packets
  if (NuPacket.isConnected()) {
    // Process chunking if in progress
    if (chunkState.inProgress) {
      int chunksToSend = min(chunkState.chunksPerIteration,
                             chunkState.totalChunks - chunkState.chunksSent);

      setIconStatus(ICON_UP, true);
      setIconStatus(ICON_DOWN, false);

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

        if (chunkState.chunksSent >= chunkState.totalChunks) {
          chunkState.inProgress = false;
          setIconStatus(ICON_UP, false);
          break;
        }
      }
    } else {
      setIconStatus(ICON_UP, false);
      // Handle incoming data - THIS PART WAS MISSING
      size_t packetSize;
      const uint8_t *data = NuPacket.read(packetSize);

      if (data && packetSize > 0) {
        setIconStatus(ICON_DOWN, true);
        // Create a null-terminated string from the received data
        char *command = (char *)malloc(packetSize + 1);
        if (command) {
          memcpy(command, data, packetSize);
          command[packetSize] = '\0';

          // Buffer for response
          uint8_t responseBuffer[200]; // Use smaller buffer size
          size_t responseSize = 0;
          if (!chunkState.inProgress) {
            // Process the command and generate a response
            processCommand(command, responseBuffer, responseSize);

            // Send the response if there is one and we haven't already sent
            // it via chunking
            if (responseSize > 0) {
              NuPacket.write(responseBuffer, responseSize);
            }
          }
          free(command);
        }
      }
    }
  }
}

void processCommand(const char *command, uint8_t *responseBuffer,
                    size_t &responseSize) {
  // Parse JSON command
  StaticJsonDocument<2048> doc;
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
  } else if (strcmp(cmd, "dumpOne") == 0) {
    handled = handlePassDumpOneCommand(doc, responseBuffer, responseSize);
  } else if (strcmp(cmd, "restore") == 0) {
    handled = handleRestoreCommand(doc, responseBuffer, responseSize);
  } else if (strcmp(cmd, "restoreOne") == 0) {
    handled = handleRestoreOneCommand(doc, responseBuffer, responseSize);
  }

  if (!handled) {
    sendResponse(404, "Unknown command", responseBuffer, responseSize);
  }
}
// API Handler implementations
void sendChunkedResponse(const char *data, size_t dataLength,
                         uint8_t *responseBuffer, size_t &responseSize) {
  // Check if chunking is already in progress
  if (chunkState.inProgress) {
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

  setIconStatus(ICON_UP, true);

  // Initialize chunking state
  chunkState.inProgress = true;
  chunkState.data = persistentChunkData.c_str(); // Point to our persistent copy
  chunkState.dataLength = dataLength;
  chunkState.offset = 0;
  chunkState.totalChunks = (dataLength + CHUNK_SIZE - 1) / CHUNK_SIZE;
  chunkState.chunksSent = 0;
  chunkState.chunksPerIteration = CHUNKS_PER_ITERATION;

  // Send header
  char header[256];
  snprintf(header, sizeof(header), "%d,%d,%d\n", dataLength,
           chunkState.totalChunks, CHUNK_SIZE);
  NuPacket.write((uint8_t *)header, strlen(header));

  // Set responseSize to 0 since we handled sending
  responseSize = 0;
}

void sendResponse(int status, const char *message, uint8_t *responseBuffer,
                  size_t &responseSize) {
  StaticJsonDocument<256> doc;
  doc["s"] = status;
  doc["m"] = message;

  // Create a temporary buffer for serialization
  char jsonBuffer[JSON_BUFFER_SIZE];
  size_t jsonSize = serializeJson(doc, jsonBuffer, JSON_BUFFER_SIZE);

  // If the response is small enough, send it directly
  if (jsonSize <= CHUNK_SIZE) {
    // Copy to the provided response buffer for direct sending
    memcpy(responseBuffer, jsonBuffer, jsonSize);
    responseSize = jsonSize;
  } else {
    // For larger responses, use chunking
    sendChunkedResponse(jsonBuffer, jsonSize, responseBuffer, responseSize);
  }
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

bool handlePassDumpOneCommand(JsonDocument &doc, uint8_t *responseBuffer,
                              size_t &responseSize) {
  // If chunking is in progress, return an error
  if (chunkState.inProgress) {
    sendResponse(503, "Transfer already in progress", responseBuffer,
                 responseSize);
    return true;
  }

  // Create a temporary String to hold the plain text dump
  StringStreamAdapter stream;
  if (dumpOnePassword(doc["uid"].as<int>(), stream)) {
    sendChunkedResponse(stream.c_str(), stream.length(), responseBuffer,
                        responseSize);
  } else {
    sendChunkedResponse("", 1, responseBuffer, responseSize);
  }
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

  // Create a temporary String to hold the plain text dump
  StringStreamAdapter stream;
  dumpPasswords(&stream);
  sendChunkedResponse(stream.c_str(), stream.length(), responseBuffer,
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

bool handleRestoreOneCommand(JsonDocument &doc, uint8_t *responseBuffer,
                             size_t &responseSize) {
  if (!doc.containsKey("data")) {
    sendResponse(400, "Missing 'data' parameter", responseBuffer, responseSize);
    return true;
  }

  const int slotIndex = doc["uid"].as<int>();
  String uploadData = doc["data"];
  restoreSinglePassword(uploadData, savePassData, slotIndex);

  sendResponse(200, "ok", responseBuffer, responseSize);
  return true;
}
