#include "configuration.h"
#if ENABLE_BLUETOOTH
#include "bluetooth.h"
#include "password.h"
#include <NimBLEDevice.h>

extern void ping();

#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define LIST_CHAR_UUID "12345678-1234-1234-1234-123456789abd"
#define FETCH_CHAR_UUID "12345678-1234-1234-1234-123456789abe"
#define EDIT_CHAR_UUID "12345678-1234-1234-1234-123456789abf"
#define TYPE_CHAR_UUID "12345678-1234-1234-1234-123456789ac0"

static bool deviceConnected = false;

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer) {
    deviceConnected = true;
    ping();
  };

  void onDisconnect(NimBLEServer *pServer) {
    deviceConnected = false;
  }
};

class ListCallback : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic *pCharacteristic) {
    ping();
    String passwords = listPasswords();
    pCharacteristic->setValue(passwords.c_str());
  }
};

class FetchCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    ping();
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0 && deviceConnected) {
      int id = atoi(value.c_str());
      const char *password = fetchPassword(id);
      if (password) {
        pCharacteristic->setValue(password);
      } else {
        pCharacteristic->setValue("Invalid ID");
      }
    }
  }
};

class EditCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    ping();
    std::string value = pCharacteristic->getValue();
    // Expecting "id,name,password,layout"
    if (value.length() > 0 && deviceConnected) {
      int id = atoi(strtok((char *)value.c_str(), ","));
      char *name = strtok(NULL, ",");
      char *password = strtok(NULL, ",");
      int layout = atoi(strtok(NULL, ","));
      vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay
      if (editPassword(id, name, password, layout)) {
        pCharacteristic->setValue("OK");
        pCharacteristic->notify();
      } else {
        pCharacteristic->setValue("Error");
        pCharacteristic->notify();
      }
    }
  }
};

class TypeCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    ping();
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0 && deviceConnected) {
      int id = atoi(value.c_str());
      vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay
      if (typePassword(id, -1, true)) {
        pCharacteristic->setValue("OK");
        pCharacteristic->notify();
      } else {
        pCharacteristic->setValue("Error");
        pCharacteristic->notify();
      }
    }
  }
};

void bluetoothSetup() {
  BLEDevice::init("KeyPass");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  // List Passwords
  BLECharacteristic *pListChar = pService->createCharacteristic(
      LIST_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  pListChar->setCallbacks(new ListCallback());
  pListChar->createDescriptor("2901", NIMBLE_PROPERTY::READ)
      ->setValue("List Passwords");

  // Fetch Password
  BLECharacteristic *pFetchChar = pService->createCharacteristic(
      FETCH_CHAR_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
  pFetchChar->setCallbacks(new FetchCallback());
  pFetchChar->createDescriptor("2901", NIMBLE_PROPERTY::READ)
      ->setValue("Fetch Password by ID");

  // Edit Password
  BLECharacteristic *pEditChar = pService->createCharacteristic(
      EDIT_CHAR_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  pEditChar->setCallbacks(new EditCallback());
  pEditChar->createDescriptor("2901", NIMBLE_PROPERTY::READ)
      ->setValue("Edit Password by ID");

  // Type Password
  BLECharacteristic *pTypeChar = pService->createCharacteristic(
      TYPE_CHAR_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  pTypeChar->setCallbacks(new TypeCallback());
  pTypeChar->createDescriptor("2901", NIMBLE_PROPERTY::READ)
      ->setValue("Type Password by ID");

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

  BLEAdvertisementData advertisementData = BLEAdvertisementData();
  advertisementData.setName("KeyPass");
  advertisementData.setFlags(0x06); // GENERAL_DISC_MODE | BR_EDR_NOT_SUPPORTED
  advertisementData.addServiceUUID(SERVICE_UUID);

  pAdvertising->setAdvertisementData(advertisementData);
  pAdvertising->start();
}

void bluetoothLoop() {
  // Nothing to do here with NimBLE
}

#endif // ENABLE_BLUETOOTH

