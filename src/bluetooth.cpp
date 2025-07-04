#include "bluetooth.h"
#include "password.h"
#include <NimBLEDevice.h>

#if ENABLE_BLUETOOTH

#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define LIST_CHAR_UUID "12345678-1234-1234-1234-123456789abd"
#define FETCH_CHAR_UUID "12345678-1234-1234-1234-123456789abe"
#define EDIT_CHAR_UUID "12345678-1234-1234-1234-123456789abf"
#define TYPE_CHAR_UUID "12345678-1234-1234-1234-123456789ac0"

class ListCallback : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic *pCharacteristic) {
    String passwords = listPasswords();
    pCharacteristic->setValue(passwords.c_str());
  }
};

class FetchCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
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
    std::string value = pCharacteristic->getValue();
    // Expecting "id,name,password,layout"
    // Simplified parsing, assuming well-formed input
    int id = atoi(strtok((char *)value.c_str(), ","));
    char *name = strtok(NULL, ",");
    char *password = strtok(NULL, ",");
    int layout = atoi(strtok(NULL, ","));
    if (editPassword(id, name, password, layout)) {
      pCharacteristic->setValue("OK");
    } else {
      pCharacteristic->setValue("Error");
    }
  }
};

class TypeCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      int id = atoi(value.c_str());
      if (typePassword(id, -1, true)) {
        // Cannot provide feedback here as it's a write characteristic
      }
    }
  }
};

void bluetoothSetup() {
  BLEDevice::init("KeyPass");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // List Passwords
  BLECharacteristic *pListChar = pService->createCharacteristic(
      LIST_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  pListChar->setCallbacks(new ListCallback());

  // Fetch Password
  BLECharacteristic *pFetchChar =
      pService->createCharacteristic(FETCH_CHAR_UUID, NIMBLE_PROPERTY::WRITE);
  pFetchChar->setCallbacks(new FetchCallback());

  // Edit Password
  BLECharacteristic *pEditChar =
      pService->createCharacteristic(EDIT_CHAR_UUID, NIMBLE_PROPERTY::WRITE);
  pEditChar->setCallbacks(new EditCallback());

  // Type Password
  BLECharacteristic *pTypeChar =
      pService->createCharacteristic(TYPE_CHAR_UUID, NIMBLE_PROPERTY::WRITE);
  pTypeChar->setCallbacks(new TypeCallback());

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