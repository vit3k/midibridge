#include <Arduino.h>

#define MIDI_DIN

#include "utils.h"
#ifdef MIDI_DIN
#include <MIDI.h>
#endif
#include "usbmidi.h"
#include "blemidi.h"

Usb::Midi usbMidi;
Ble::Midi bleMidi;

#ifdef MIDI_DIN
struct MidiSettings : public midi::DefaultSettings
{
    static const long BaudRate = 115200;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, MidiSettings);

void sendMidi(uint8_t* data, uint8_t size)
{
  if(data[0] == 0xF0) {
    MIDI.sendSysEx(size, data, true);
  } else if (data[0] == 0xF8) {
    MIDI.sendRealTime((midi::MidiType)data[0]);
  } else if (size == 2) {
    MIDI.send((midi::MidiType)(data[0] & 0xF0), data[1], 0, (midi::Channel)(data[0] & 0x0F + 1));
  } else {
    MIDI.send((midi::MidiType)(data[0] & 0xF0), data[1], data[2], (midi::Channel)(data[0] & 0x0F +1));
  }
}
void updateMidi()
{
    if(MIDI.read())
    {
      auto channel = MIDI.getChannel();
      auto type = MIDI.getType();
      auto status = type | channel;
      //TODO: more message handling
      switch(type)
      {
        case midi::SystemExclusive:
        {
          auto data = MIDI.getSysExArray();
          usbMidi.send((byte*)data);
          bleMidi.send((byte*)data, MIDI.getSysExArrayLength());
          break;
        }
        case midi::ProgramChange:
        {
          byte data[2] = {status, MIDI.getData1()};
          usbMidi.send(data);
          bleMidi.send(data, 2);
          break;
        }
        case midi::ControlChange:
        {
          byte data[3] = {status, MIDI.getData1(), MIDI.getData2()};
          usbMidi.send(data);
          bleMidi.send(data, 3);
          break;
        }
      }
    }
}
#endif

void OnUSBMidiMessage(uint8_t* data, uint8_t size) {
  Utils::printHex(data, size);
  bleMidi.send(data, size);
  #ifdef MIDI_DIN
  sendMidi(data, size);
  #endif
}

void OnBLEMidiMessage(uint8_t* data, uint8_t size) {
  Utils::printHex(data, size);
  usbMidi.send(data);
  #ifdef MIDI_DIN
  sendMidi(data, size);
  #endif
}

void setup() {
  Serial.begin(115200);
  usbMidi.setup();
  usbMidi.registerCallback(OnUSBMidiMessage);
  bleMidi.setup(OnBLEMidiMessage);
  Serial.println("Setup completed");
}

void loop() {
    usbMidi.update();
    #ifdef MIDI_DIN
    updateMidi();
    #endif
}