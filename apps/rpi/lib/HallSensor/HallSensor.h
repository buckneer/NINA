#pragma once
#include <Arduino.h>

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

    volatile uint32_t _pulseCount = 0;
    float             _speedKph   = 0.0f;
    uint32_t          _lastMs     = 0;
    uint32_t          _lastSnap   = 0;

    static HallSensor* _instance;
};
