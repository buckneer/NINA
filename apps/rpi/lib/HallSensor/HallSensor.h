#pragma once
#include <Arduino.h>

// Minimum microseconds between counted pulses.
// At 3200 pulses/mile and 200 km/h the real max rate is ~110 Hz (≈9 ms apart).
// 3 ms rejects sensor oscillation while comfortably passing genuine pulses.
static constexpr uint32_t HALL_DEBOUNCE_US = 3000;

class HallSensor {
public:
    HallSensor(uint8_t pin, float metersPerPulse, uint16_t sampleMs = 200);

    void  begin();
    void  update();
    float speedKph() const { return _speedKph; }

    static void isr();

private:
    uint8_t  _pin;
    float    _metersPerPulse;
    uint16_t _sampleMs;

    volatile uint32_t _pulseCount   = 0;
    volatile uint32_t _lastPulseUs  = 0;
    float             _speedKph     = 0.0f;
    uint32_t          _lastMs       = 0;
    uint32_t          _lastSnap     = 0;

    static HallSensor* _instance;
};
