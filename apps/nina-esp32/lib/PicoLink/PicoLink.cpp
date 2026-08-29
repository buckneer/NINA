#include "PicoLink.h"

PicoLink::PicoLink(uint8_t addr)
    : _addr(addr)
{
}

bool PicoLink::scanForAddr(
    TwoWire &bus,
    const char *label,
    uint8_t addr)
{
    Serial.printf("[PicoLink] Scanning %s...\n", label);

    bool found = false;

    for (uint8_t a = 1; a < 127; a++)
    {
        bus.beginTransmission(a);

        if (bus.endTransmission() == 0)
        {
            Serial.printf(
                "[PicoLink] %s: found 0x%02X\n",
                label,
                a);

            if (a == addr)
                found = true;
        }

        delay(2);
    }

    return found;
}

void PicoLink::begin(TwoWire &bus0, TwoWire &bus1)
{
    const bool onBus1 =
        scanForAddr(bus1, "bus1", _addr);

    const bool onBus0 =
        onBus1
            ? false
            : scanForAddr(bus0, "bus0", _addr);

    if (onBus1)
    {
        _wire = &bus1;

        Serial.printf(
            "[PicoLink] Using bus1 (0x%02X)\n",
            _addr);
    }
    else if (onBus0)
    {
        _wire = &bus0;

        Serial.printf(
            "[PicoLink] Using bus0 (0x%02X)\n",
            _addr);
    }
    else
    {
        _wire = &bus1;

        Serial.printf(
            "[PicoLink] WARNING: 0x%02X not found, defaulting to bus1\n",
            _addr);
    }
}

void PicoLink::begin(TwoWire &wire)
{
    _wire = &wire;
}

void PicoLink::update()
{
    if (!_wire)
        return;

    const unsigned long now = millis();

    if (now - _lastPoll < POLL_MS)
        return;

    _lastPoll = now;

    const uint8_t got =
        _wire->requestFrom(_addr, PACKET_LEN);

    if (got == PACKET_LEN)
    {
        uint8_t p[PACKET_LEN];

        for (uint8_t i = 0; i < PACKET_LEN; i++)
            p[i] = _wire->read();

        uint8_t xorAll = 0;

        for (uint8_t i = 0; i < PACKET_LEN; i++)
            xorAll ^= p[i];

        if ((p[0] ^ p[1]) != 0xFF || xorAll != 0)
        {
            Serial.println("[PicoLink] Bad packet");
            return;
        }

        // Buttons
        _buttons = p[0];

        // Speed: uint16, units of 0.1 km/h
        const uint16_t speedRaw =
            static_cast<uint16_t>(p[2]) |
            (static_cast<uint16_t>(p[3]) << 8);

        _speedKph = speedRaw / 10.0f;

        // Button hold times
        _nextHeldMs =
            static_cast<uint32_t>(p[4]) * 10;

        _okHeldMs =
            static_cast<uint32_t>(p[5]) * 10;

        // Cumulative Hall pulse counter from Pico
        _pulseCount =
            static_cast<uint32_t>(p[6]) |
            (static_cast<uint32_t>(p[7]) << 8) |
            (static_cast<uint32_t>(p[8]) << 16) |
            (static_cast<uint32_t>(p[9]) << 24);

        _hasData = true;

        // Short press edges (0 → 1)
        const uint8_t edges =
            static_cast<uint8_t>(
                (_buttons ^ _lastButtons) &
                _buttons);

        _pressedEdges |= edges;

        // Release edges (1 → 0)
        const uint8_t released =
            static_cast<uint8_t>(
                (~_buttons) &
                _lastButtons);

        if (released & (1u << 0))
        {
            _nextLongFired = false;
            _nextLastTick = 0;
        }

        if (released & (1u << 1))
        {
            _okLongFired = false;
            _okLastTick = 0;
        }

        _lastButtons = _buttons;

        if (now - _lastLog >= LOG_MS)
        {
            _lastLog = now;

            Serial.printf(
                "[PicoLink] speed=%.1f km/h  pulses=%lu  btn=0x%02X  hold=[%lu,%lu]ms\n",
                _speedKph,
                static_cast<unsigned long>(_pulseCount),
                _buttons,
                static_cast<unsigned long>(_nextHeldMs),
                static_cast<unsigned long>(_okHeldMs));
        }
    }
    else if (got > 0)
    {
        while (_wire->available())
            (void)_wire->read();
    }
}

bool PicoLink::nextPressed()
{
    if (_pressedEdges & (1u << 0))
    {
        _pressedEdges &= ~(1u << 0);
        return true;
    }

    return false;
}

bool PicoLink::okPressed()
{
    if (_pressedEdges & (1u << 1))
    {
        _pressedEdges &= ~(1u << 1);
        return true;
    }

    return false;
}

bool PicoLink::nextLongPressed(uint32_t thresholdMs)
{
    if (!_nextLongFired &&
        _nextHeldMs >= thresholdMs)
    {
        _nextLongFired = true;
        return true;
    }

    return false;
}

bool PicoLink::okLongPressed(uint32_t thresholdMs)
{
    if (!_okLongFired &&
        _okHeldMs >= thresholdMs)
    {
        _okLongFired = true;
        return true;
    }

    return false;
}

bool PicoLink::accelTick(
    uint32_t heldMs,
    uint32_t startMs,
    uint32_t slowMs,
    uint32_t fastMs,
    unsigned long &lastTick)
{
    if (heldMs < startMs)
    {
        lastTick = 0;
        return false;
    }

    const uint32_t elapsed =
        heldMs - startMs;

    const uint32_t rampMs = 2000;

    const uint32_t interval =
        (elapsed >= rampMs)
            ? fastMs
            : slowMs -
                  ((slowMs - fastMs) * elapsed / rampMs);

    const unsigned long now = millis();

    if (lastTick == 0 ||
        (now - lastTick) >= interval)
    {
        lastTick = now;
        return true;
    }

    return false;
}

bool PicoLink::nextAccelTick(
    uint32_t startMs,
    uint32_t slowMs,
    uint32_t fastMs)
{
    return accelTick(
        _nextHeldMs,
        startMs,
        slowMs,
        fastMs,
        _nextLastTick);
}

bool PicoLink::okAccelTick(
    uint32_t startMs,
    uint32_t slowMs,
    uint32_t fastMs)
{
    return accelTick(
        _okHeldMs,
        startMs,
        slowMs,
        fastMs,
        _okLastTick);
}