#include <Arduino.h>
#include "usbmidi.h"

class Delay
{
private:
    uint8_t* time;
    uint8_t* feedback;
    uint8_t* level;
    uint8_t* modulationRate;
    uint8_t* modulationDepth;
    uint8_t* highCut;
    uint8_t* dryMix;
public:
    Delay(uint8_t* time): time(time) {}
    uint16_t getTime() { return (*time << 8) | *(time+1);  }
};

class Katana : public Usb::MidiMessageCallbackReceiver
{
    uint8_t memory[6000];
    Usb::Midi& midi;
public:
    Delay delay;
    Katana(Usb::Midi& midi): midi(midi), delay(&memory[0x0562]) {}
    void update();
    void init();
    void set(uint32_t address, uint16_t data, uint8_t size);
    void query(uint32_t address, uint8_t size);
    void sendCommand(uint32_t address, uint8_t command, uint32_t data, uint8_t dataSize);
    uint8_t calculateChecksum(uint8_t* vals, uint8_t valsSize);
    void receive(uint8_t* data, uint8_t size);
};
