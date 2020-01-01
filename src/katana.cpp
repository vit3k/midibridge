#include "katana.h"

void Katana::init() {
    //byte addr[4] = {0x7F, 0x00, 0x00, 0x01};
    midi.registerCallback(this);
    set(0x7F000001, 0x01, 1);

    // TODO: query all data for preset
}
uint8_t Katana::calculateChecksum(uint8_t* vals, uint8_t valsSize)
{
    byte acc = 0;
    for(byte i = 0; i < valsSize; i++)
    {
        acc = (acc + vals[i]) & 0x7F;
    }

    return (128 - acc) & 0x7F;
}

void Katana::sendCommand(uint32_t address, uint8_t command, uint32_t data, uint8_t dataSize)
{
    byte rawData[8] = {(address >> 24) & 0xFF, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF};
    for(byte i = 0; i < dataSize; i++)
    {
        rawData[4 + i] = data >> (8 * (dataSize - i - 1)) & 0xFF;
    }
    byte size = 4 + dataSize;
    byte sysexData[20] = {0xF0, 0x41, 0x00, 0x00, 0x00, 0x00, 0x33, command};
    memcpy(sysexData + 8, rawData, size);
    sysexData[8 + size] = calculateChecksum(rawData, size);
    sysexData[8 + size + 1] = 0xF7;
    midi.send(sysexData);
}

void Katana::set(uint32_t address, uint16_t data, uint8_t size)
{
    sendCommand(0x12, address, data, size);
}
void Katana::query(uint32_t address, uint8_t size)
{
    sendCommand(0x11, address, (uint32_t)size, 4);
}

void Katana::update()
{

}

void Katana::receive(uint8_t* rawData, uint8_t size)
{
    if (rawData[0] != 0xF0)
    {
        return;
    }
    uint32_t address = (rawData[8] << 24) | (rawData[9] << 16) | (rawData[10] << 8) | rawData[11];
    if (address < 0x60000000 && address > 0x60001569)
    {
        return;
    }
    auto idx = (uint16_t)(address & 0xFFFF);
    //idx = ((idx / 0x7F) << 8) | (idx & 0x7F);
    memcpy(&memory[idx], &rawData[12], size - 14);
}