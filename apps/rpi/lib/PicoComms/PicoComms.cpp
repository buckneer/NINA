#include "PicoComms.h"

PicoComms *PicoComms::_instance = nullptr;

void PicoComms::onRequest()
{
    if (_instance)
    {
        Wire.write(
            const_cast<uint8_t *>(_instance->_packet),
            PACKET_LEN);
    }
}

PicoComms::PicoComms(
    uint8_t addr,
    uint32_t freqHz)
    : _addr(addr),
      _freq(freqHz)
{
}

void PicoComms::begin()
{
    _instance = this;

    Wire.begin(_addr);
    Wire.setClock(_freq);

    Wire.onRequest(
        PicoComms::onRequest);
}

void PicoComms::send(
    uint8_t buttons,
    float speedKph,
    uint32_t pulseCount,
    uint32_t nextHoldMs,
    uint32_t okHoldMs)
{
    const uint16_t speedRaw =
        static_cast<uint16_t>(
            speedKph * 10.0f + 0.5f);

    const uint8_t nextHoldRaw =
        static_cast<uint8_t>(
            min(
                nextHoldMs / 10,
                static_cast<uint32_t>(255)));

    const uint8_t okHoldRaw =
        static_cast<uint8_t>(
            min(
                okHoldMs / 10,
                static_cast<uint32_t>(255)));

    uint8_t p[PACKET_LEN];

    // ======================
    // Buttons
    // ======================

    p[0] = buttons;
    p[1] = static_cast<uint8_t>(~buttons);

    // ======================
    // Speed
    // ======================

    p[2] =
        static_cast<uint8_t>(
            speedRaw & 0xFF);

    p[3] =
        static_cast<uint8_t>(
            (speedRaw >> 8) & 0xFF);

    // ======================
    // Button hold durations
    // ======================

    p[4] = nextHoldRaw;
    p[5] = okHoldRaw;

    // ======================
    // Cumulative Hall pulses
    // uint32_t little-endian
    // ======================

    p[6] =
        static_cast<uint8_t>(
            pulseCount & 0xFF);

    p[7] =
        static_cast<uint8_t>(
            (pulseCount >> 8) & 0xFF);

    p[8] =
        static_cast<uint8_t>(
            (pulseCount >> 16) & 0xFF);

    p[9] =
        static_cast<uint8_t>(
            (pulseCount >> 24) & 0xFF);

    // ======================
    // Checksum
    // ======================

    uint8_t checksum = 0;

    for (uint8_t i = 0; i < PACKET_LEN - 1; i++)
    {
        checksum ^= p[i];
    }

    p[10] = checksum;

    // ======================
    // Publish packet
    // ======================
    //
    // Prevent the I2C request callback from reading
    // the packet halfway through an update.
    //

    noInterrupts();

    for (uint8_t i = 0; i < PACKET_LEN; i++)
    {
        _packet[i] = p[i];
    }

    interrupts();
}