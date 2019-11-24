#ifndef H_BLEMIDI
#define H_BLEMIDI

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "bleparser.h"

#define SERVICE_UUID        "03B80E5A-EDE8-4B33-A751-6CE34EC4C700"
#define CHARACTERISTIC_UUID "7772E5DB-3868-4112-A1A9-F2669D106BF3"

namespace Ble {
    using MidiMessageCallback = void(*)(uint8_t*, uint8_t);

    class Midi {
        private:
            BLECharacteristic* pCharacteristic;
        public:
            void setup(MidiMessageCallback callback);
    };

    class MidiBLECallbacks: public BLECharacteristicCallbacks {
        MidiMessageCallback callback;
        BLEParser bleParser;
        void onWrite(BLECharacteristic *pCharacteristic);
        public:
            MidiBLECallbacks(MidiMessageCallback callback) {
                this->callback = callback;
            }
    };
}

#endif