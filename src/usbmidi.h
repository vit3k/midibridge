#ifndef H_USB_MIDI
#define H_USB_MIDI

#include <usbhub.h>
#include <usbh_midi.h>
#define ENABLE_UHS_DEBUGGING 1
namespace Usb {

    using MidiMessageCallback = void(*)(uint8_t*, uint8_t);

    class USBH_MIDI_ext : public USBH_MIDI {
    public:
        byte port;
        byte Init(byte parent, byte port, bool lowspeed);
        USBH_MIDI_ext(USB* usb): USBH_MIDI(usb) {};
        virtual ~USBH_MIDI_ext() {}
    };
    class MidiSysEx {
    private:
            uint8_t pos;
            uint8_t buf[MIDI_EVENT_PACKET_SIZE];
    public:
            typedef enum {
                    nonsysex = 0,
                    ok       = 1,
                    done     = 0xfe,
                    overflow = 0xff
            } Status;

            MidiSysEx();
            void clear();
            MidiSysEx::Status set(uint8_t *p);
            inline uint8_t *get(){return buf;};
            inline uint8_t getSize(){return pos;};
    };

    class Midi {
        USBH_MIDI_ext* midi;
        USB usb;
        MidiSysEx sysEx;
        void poll();
        MidiMessageCallback callback;
    public:
        Midi(): midi(new USBH_MIDI_ext(&usb)) {};
        ~Midi() { delete midi; }
        void setup();
        void update();
        void send(byte* data);
        void registerCallback(MidiMessageCallback callback) { this->callback = callback; };
        byte getPort() { return midi->port; }
    };
};
#endif