#include "PicoLink.h"

PicoLink::PicoLink(uint8_t addr) : _addr(addr) {}

bool PicoLink::scanForAddr(TwoWire& bus, const char* label, uint8_t addr) {
    Serial.printf("[PicoLink] Scanning %s...\n", label);
    bool found = false;
    for (uint8_t a = 1; a < 127; a++) {
        bus.beginTransmission(a);
        if (bus.endTransmission() == 0) {
            Serial.printf("[PicoLink] %s: found 0x%02X\n", label, a);
            if (a == addr) found = true;
        }
        delay(2);
    }
    return found;
}

void PicoLink::begin(TwoWire& bus0, TwoWire& bus1) {
    const bool onBus1 = scanForAddr(bus1, "bus1", _addr);
    const bool onBus0 = onBus1 ? false : scanForAddr(bus0, "bus0", _addr);

    if (onBus1)       { _wire = &bus1; Serial.printf("[PicoLink] Using bus1 (0x%02X)\n", _addr); }
    else if (onBus0)  { _wire = &bus0; Serial.printf("[PicoLink] Using bus0 (0x%02X)\n", _addr); }
    else              { _wire = &bus1; Serial.printf("[PicoLink] WARNING: 0x%02X not found, defaulting to bus1\n", _addr); }
}

void PicoLink::begin(TwoWire& wire) {
    _wire = &wire;
}

void PicoLink::update() {
    if (!_wire) return;

    const unsigned long now = millis();
    if (now - _lastPoll < POLL_MS) return;
    _lastPoll = now;

    const uint8_t got = _wire->requestFrom(_addr, PACKET_LEN);
    if (got == PACKET_LEN) {
        uint8_t p[PACKET_LEN];
        for (uint8_t i = 0; i < PACKET_LEN; i++) p[i] = _wire->read();

        uint8_t xorAll = 0;
        for (uint8_t i = 0; i < PACKET_LEN; i++) xorAll ^= p[i];

        if ((p[0] ^ p[1]) != 0xFF || xorAll != 0) {
            Serial.println("[PicoLink] Bad packet");
            return;
        }

        _buttons  = p[0];
        _speedKph = static_cast<uint16_t>(p[2] | (static_cast<uint16_t>(p[3]) << 8)) / 10.0f;

        const uint8_t edges = static_cast<uint8_t>((_buttons ^ _lastButtons) & _buttons);
        _pressedEdges |= edges;
        _lastButtons   = _buttons;

        if (now - _lastLog >= LOG_MS) {
            _lastLog = now;
            Serial.printf("[PicoLink] speed=%.1f km/h  btn=0x%02X\n", _speedKph, _buttons);
        }
    } else if (got > 0) {
        while (_wire->available()) (void)_wire->read();
    }
}

bool PicoLink::nextPressed() {
    if (_pressedEdges & (1u << 0)) { _pressedEdges &= ~(1u << 0); return true; }
    return false;
}

bool PicoLink::okPressed() {
    if (_pressedEdges & (1u << 1)) { _pressedEdges &= ~(1u << 1); return true; }
    return false;
}
