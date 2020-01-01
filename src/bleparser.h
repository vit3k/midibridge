#ifndef H_BLEPARSER
#define H_BLEPARSER

#include <Arduino.h>

namespace Ble {
//void (* MessageCallback)(uint8_t*, uint8_t);
    using MidiMessageCallback = void(*)(uint8_t*, uint8_t);
    class MidiMessageCallbackReceiver
    {
        public:
            virtual void receive(uint8_t* data, uint8_t size) = 0;
    };
    class BLEParser {
    public:
        bool sysex = false;
        uint8_t sysexData[300];
        uint8_t sysexIdx = 0;

        void parseBLE(uint8_t* data, uint8_t dataSize, MidiMessageCallbackReceiver* receiver );
    };

}
#endif
