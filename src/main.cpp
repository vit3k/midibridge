#include <Arduino.h>

#include "utils.h"
#include <MIDI.h>
#include "usbmidi.h"

struct MidiSettings : public midi::DefaultSettings
{
    static const long BaudRate = 115200;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, MidiSettings);

Usb::Midi usbMidi;

// void OnBLEMidiMessage(uint8_t* data, uint8_t size) {
//   Utils::printHex(data, size);
//   usbMidi.send(data);
//   if(data[0] == 0xF0) {
//     MIDI.sendSysEx(size, data, true);
//   } else if (data[0] == 0xF8) {
//     MIDI.sendRealTime((midi::MidiType)data[0]);
//   } else if (size == 2) {
//     MIDI.send((midi::MidiType)(data[0] & 0xF0), data[1], 0, (midi::Channel)(data[0] & 0x0F + 1));
//   } else {
//     MIDI.send((midi::MidiType)(data[0] & 0xF0), data[1], data[2], (midi::Channel)(data[0] & 0x0F +1));
//   }
// }

void OnUSBMidiMessage(uint8_t* data, uint8_t size) {
  Utils::printHex(data, size);
  //bleMidi.send(data, size);
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
void setup() {
    usbMidi.setup();
    usbMidi.registerCallback(OnUSBMidiMessage);
    //bleMidi.setup(OnBLEMidiMessage);
}

void loop() {
    usbMidi.update();
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
          break;
        }
        case midi::ProgramChange:
        {
          byte data[2] = {status, MIDI.getData1()};
          usbMidi.send(data);
          break;
        }
        case midi::ControlChange:
        {
          byte data[3] = {status, MIDI.getData1(), MIDI.getData2()};
          usbMidi.send(data);
          break;
        }
      }
    }
}