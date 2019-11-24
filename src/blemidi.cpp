#include "blemidi.h"
#include "utils.h"

namespace Ble {

    void MidiBLECallbacks::onWrite(BLECharacteristic *pCharacteristic) {
      auto value = pCharacteristic->getValue();
      if (value.length() > 0) {
        Utils::printHex((byte*)value.data(), value.length());
        auto data = (uint8_t*)value.data();
        auto dataSize = value.length();
        bleParser.parseBLE(data, dataSize, callback);
        pCharacteristic->setValue(nullptr, 0);
        pCharacteristic->notify();
      }
    }

    void Midi::setup(MidiMessageCallback callback) {
        BLEDevice::init("MidiBridge");
        BLEDevice::setEncryptionLevel((esp_ble_sec_act_t)ESP_LE_AUTH_REQ_SC_BOND);

        auto pServer = BLEDevice::createServer();

        auto pService = pServer->createService(SERVICE_UUID);

        pCharacteristic = pService->createCharacteristic(
                                            CHARACTERISTIC_UUID,
                                            BLECharacteristic::PROPERTY_READ |
                                            BLECharacteristic::PROPERTY_WRITE_NR |
                                            BLECharacteristic::PROPERTY_NOTIFY
                                            );
        pCharacteristic->setCallbacks(new MidiBLECallbacks(callback));
        pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);

        uint8_t init[2] = {0x80, 0x80};
        pCharacteristic->setValue(init, 2);
        pService->start();

        auto pSecurity = new BLESecurity();
        pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
        pSecurity->setCapability(ESP_IO_CAP_NONE);
        pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

        pServer->getAdvertising()->addServiceUUID(SERVICE_UUID);
        pServer->getAdvertising()->start();
    }
}