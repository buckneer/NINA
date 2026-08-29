#pragma once

#include <Arduino.h>

// Ignore pulses closer than 3 ms.
// Real speedometer pulses should never legitimately be this close.
static constexpr uint32_t HALL_DEBOUNCE_US = 3000;

// If we have received no pulse for this long, consider the car stopped.
// 1.5 seconds gives good low-speed behavior without leaving a speed
// displayed for too long after stopping.
static constexpr uint32_t HALL_STOP_TIMEOUT_US = 1500000;

class HallSensor
{
public:
    HallSensor(
        uint8_t pin,
        float metersPerPulse,
        uint16_t sampleMs = 50);

    void begin();
    void update();

    // Return the actual filtered speed.
    // No rounding to multiples of 5.
    float speedKph() const
    {
        return _speedKph;
    }

    uint32_t pulseCount() const
    {
        return _pulseCount;
    }

    static void isr();

private:
    uint8_t _pin;
    float _metersPerPulse;
    uint16_t _sampleMs;

    volatile uint32_t _pulseCount = 0;
    volatile uint32_t _lastPulseUs = 0;
    volatile uint32_t _lastPeriodUs = 0;

    uint32_t _lastProcessedPulseCount = 0;
    uint32_t _lastUpdateMs = 0;

    float _speedKph = 0.0f;

    static HallSensor *_instance;
};