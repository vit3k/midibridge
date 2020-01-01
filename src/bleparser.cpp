#include "bleparser.h"
#include "utils.h"

namespace Ble {
    bool isBitOn(uint8_t value, uint8_t bit) {
        return (value & (1 << bit)) != 0;
    }

    void BLEParser::parseBLE(uint8_t* data, uint8_t dataSize, MidiMessageCallbackReceiver* receiver) {
        uint8_t command;
        uint8_t channel;
        uint8_t data1;
        uint8_t data2;
        uint8_t currentIdx = 1;
        uint8_t statusByte;
        Serial.printf("New BLE packet %d\n", dataSize);
        while(currentIdx < dataSize)
        {
            //Serial.printf("Current idx: %d\n", currentIdx);
            if (isBitOn(data[currentIdx], 7)) { // new message with timestamp
                currentIdx++;
                statusByte = data[currentIdx];
                command = statusByte >> 4;
                channel = statusByte & 0x0F;
                Serial.printf("New message: %x %x\n", command, channel);
                //currentIdx++;
            } // if there is no timestamp byte it means it is a running message or it's still sysex even from previous packet
            else if (sysex) {
                if (isBitOn(data[currentIdx], 7)) {
                    // this is real time message
                    // MIDI.send(data[currentIdx]);
                    receiver->receive(&data[currentIdx], 1);
                } else {
                    sysexData[sysexIdx++] = data[currentIdx];
                }
                currentIdx++;
            }
            if (!sysex || statusByte == 0xF7) {
                switch(command) {
                    case 0b1000:
                    case 0b1001:
                    case 0b1010:
                    case 0b1011:
                    case 0b1110: // 2 bytes of data
                        //data1 = data[currentIdx++];
                        //data2 = data[currentIdx++];
                        receiver->receive(&data[currentIdx], 3);
                        currentIdx += 3;
                        // MIDI.send(statusByte, data1, data2);
                        break;
                    case 0b1100:
                    case 0b1101: //1 byte of data
                        //data1 = data[currentIdx++];
                        // MIDI.send(statusByte, data1);
                        receiver->receive(&data[currentIdx], 2);
                        currentIdx += 2;
                        break;
                    case 0b1111: // system common or system real time
                        if (isBitOn(channel, 3)) {
                            // system real time
                            // MIDI.send(statusByte);
                            Serial.printf("MIDI real time %x %x %d\n", data[currentIdx], channel, isBitOn(channel, 3));
                            receiver->receive(&data[currentIdx], 1);
                            currentIdx++;
                        } else {
                            // system common
                            switch(channel)
                            {
                                case 0b0000:
                                    sysex = true;
                                    sysexData[sysexIdx] = data[currentIdx];
                                    currentIdx++;
                                    sysexIdx++;
                                    // sysex start
                                    break;
                                case 0b0111:
                                    sysex = false;
                                    // MIDI.sendSysex(sysexData, sysexIdx + 1);
                                    sysexData[sysexIdx] = data[currentIdx];
                                    sysexIdx++;
                                    Utils::printHex(sysexData, sysexIdx);
                                    receiver->receive(sysexData, sysexIdx);
                                    sysexIdx = 0;
                                    currentIdx++;
                                    //Serial.printf("Sysex send");
                                    // sysex end
                                    break;
                                case 0b0010: // 2 bytes of data
                                    //data1 = data[currentIdx++];
                                    //data2 = data[currentIdx++];
                                    // MIDI.send(statusByte, data1, data2);
                                    receiver->receive(&data[currentIdx], 3);
                                    currentIdx += 3;
                                    break;
                                case 0b0001:
                                case 0b0011: // 1 byte of data
                                    data1 = data[currentIdx++];
                                    receiver->receive(&data[currentIdx], 2);
                                    currentIdx += 2;
                                    // MIDI.send(statusByte, data1);
                                    break;
                                default: // 0 bytes of data
                                    break;
                            }
                        }
                        break;
                }
            }
        }
    }
}
