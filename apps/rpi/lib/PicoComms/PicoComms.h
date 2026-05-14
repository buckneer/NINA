#pragma once
#include <Arduino.h>
#include <Wire.h>

// I2C slave – sends telemetry to the ESP32 master on request.
//
// Packet (7 bytes):
//   B0 : buttons bitmask (bit0=NEXT, bit1=OK)
//   B1 : ~B0
//   B2 : speed_lo  (uint16_t LE, 0.1 km/h)
//   B3 : speed_hi
//   B4 : NEXT hold duration (10 ms units, 0 = not held, max 2550 ms)
//   B5 : OK   hold duration (10 ms units, 0 = not held, max 2550 ms)
//   B6 : XOR checksum of B0..B5

class PicoComms {
public:
    PicoComms(uint8_t addr, uint32_t freqHz = 100000);

    void begin();
    void send(uint8_t buttons, float speedKph,
              uint32_t nextHoldMs = 0, uint32_t okHoldMs = 0);

    static void onRequest();
    static constexpr uint8_t PACKET_LEN = 7;

private:
    uint8_t  _addr;
    uint32_t _freq;
    volatile uint8_t _packet[PACKET_LEN] = {};

    static PicoComms* _instance;
};
