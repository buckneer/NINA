#include "PicoComms.h"

PicoComms* PicoComms::_instance = nullptr;

void PicoComms::onRequest() {
    if (_instance)
        Wire.write(const_cast<uint8_t*>(_instance->_packet), PACKET_LEN);
}

PicoComms::PicoComms(uint8_t addr, uint32_t freqHz)
    : _addr(addr), _freq(freqHz) {}

void PicoComms::begin() {
    _instance = this;
    // Mbed-Arduino RP2040 core uses GP4/GP5 as Wire defaults (matches HardwareConfig).
    Wire.begin(_addr);
    Wire.setClock(_freq);
    Wire.onRequest(PicoComms::onRequest);
}

void PicoComms::send(uint8_t buttons, float speedKph) {
    const uint16_t speedRaw = static_cast<uint16_t>(speedKph * 10.0f + 0.5f);
    uint8_t p[PACKET_LEN];
    p[0] = buttons;
    p[1] = ~buttons;
    p[2] = static_cast<uint8_t>(speedRaw & 0xFF);
    p[3] = static_cast<uint8_t>(speedRaw >> 8);
    p[4] = p[0] ^ p[1] ^ p[2] ^ p[3];
    for (uint8_t i = 0; i < PACKET_LEN; i++) _packet[i] = p[i];
}
