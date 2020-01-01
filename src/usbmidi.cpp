#include "usbmidi.h"
namespace Usb {
    void Midi::setup() {
        if (usb.Init() == -1) {
            Serial.println("Can't init USB");
            while (1);
        }
        //usb.
        delay( 200 );
    }
    void Midi::update() {
        usb.Task();
        if (usb.getUsbTaskState() == USB_STATE_RUNNING) {
            poll();
        }
    }
    void Midi::send(byte* data) {
        if (usb.getUsbTaskState() == USB_STATE_RUNNING) {
            midi->SendData(data);
        }
    }
    void Midi::poll() {
        uint16_t bytesRcvd = 0;
        //byte size = 0;

        byte recvBuf[MIDI_EVENT_PACKET_SIZE];
        byte readPtr = 0;
        byte status = midi->RecvData(&bytesRcvd, recvBuf);

        if (status != 0) return;
        byte* p = recvBuf;
        while (readPtr < MIDI_EVENT_PACKET_SIZE)  {
            if (*p == 0 && *(p + 1) == 0) break; //data end

            MidiSysEx::Status rc = sysEx.set(p);
            byte size;
            switch(rc) {
                case MidiSysEx::nonsysex:
                    p++;
                    size = midi->lookupMsgSize(*p);
                    callCallbacks(p, size);
                    p += size;
                    break;
                case MidiSysEx::done:
                    callCallbacks(sysEx.get(), sysEx.getSize());
                    sysEx.clear();
                    break;
                case MidiSysEx::overflow:
                    sysEx.clear();
                    break;
                default:
                    p += 4;
                    break;
            }
            readPtr += 4;
        }
    }
    byte USBH_MIDI_ext::Init(byte parent, byte port, bool lowspeed)
    {
        auto rcode = USBH_MIDI::Init(parent, port, lowspeed);
        if (rcode == 0) {
            this->port = port;
        }
        return rcode;
    }

    MidiSysEx::MidiSysEx()
    {
        clear();
    }

    void MidiSysEx::clear()
    {
        pos = 0;
        buf[0] = 0;
    }

    MidiSysEx::Status MidiSysEx::set(uint8_t *p)
    {
        MidiSysEx::Status rc = MidiSysEx::ok;
        uint8_t cin = *(p) & 0x0f;

        //SysEx message?
        if( (cin & 0xc) != 4 ) return MidiSysEx::nonsysex;

        switch(cin) {
            case 4:
            case 7:
                if( pos+2 < MIDI_EVENT_PACKET_SIZE ) {
                    buf[pos++] = *(p+1);
                    buf[pos++] = *(p+2);
                    buf[pos++] = *(p+3);
                } else{
                    rc = MidiSysEx::overflow;
                }
                break;
            case 5:
                if( pos+1 < MIDI_EVENT_PACKET_SIZE ) {
                    buf[pos++] = *(p+1);
                    buf[pos++] = *(p+2);
                }else{
                    rc = MidiSysEx::overflow;
                }
                break;
            case 6:
                if( pos < MIDI_EVENT_PACKET_SIZE ) {
                    buf[pos++] = *(p+1);
                } else{
                    rc = MidiSysEx::overflow;
                }
                break;
            default:
                break;
        }
        //SysEx end?
        if((cin & 0x3) != 0) {
            rc = MidiSysEx::done;
        }
        return(rc);
    }

    void Midi::registerCallback(MidiMessageCallbackReceiver* callback)
    {
        receivers[callbackSize++] = callback;
    }

    void Midi::callCallbacks(uint8_t* data, uint8_t size)
    {
        for(auto i = 0; i < callbackSize; i++)
        {
            receivers[i]->receive(data, size);
        }
    }
}