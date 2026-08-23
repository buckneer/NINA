#include "HallSensor.h"

HallSensor *HallSensor::_instance = nullptr;

void HallSensor::isr()
{
    if (!_instance)
        return;
    const uint32_t now = micros();
    if (now - _instance->_lastPulseUs >= HALL_DEBOUNCE_US)
    {
        _instance->_pulseCount++;
        _instance->_lastPulseUs = now;
    }
}

HallSensor::HallSensor(uint8_t pin, float metersPerPulse, uint16_t sampleMs)
    : _pin(pin), _metersPerPulse(metersPerPulse), _sampleMs(sampleMs) {}

void HallSensor::begin()
{
    _instance = this;
    _lastMs = millis();
    pinMode(_pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_pin), HallSensor::isr, FALLING);
}

void HallSensor::update()
{
    const uint32_t now = millis();
    if (now - _lastMs < _sampleMs)
        return;

    const uint32_t pulses = _pulseCount - _lastSnap;
    const float dtSec = (now - _lastMs) / 1000.0f;
    _lastSnap = _pulseCount;
    _lastMs = now;

    if (pulses == 0)
    {
        _speedKph = 0.0f;
        return;
    }

    const float raw = (pulses * _metersPerPulse / dtSec) * 3.6f;

    // Reject implausible jump from standstill to near-max — catches 50 Hz
    // sensor threshold oscillation which reads as ~181 km/h.
    if (_speedKph < 5.0f && raw > 160.0f)
        return;

    // Asymmetric EMA: slow to rise (filters noise), fast to fall (real decel).
    const float alpha = (raw < _speedKph) ? 0.5f : 0.2f;
    _speedKph = alpha * raw + (1.0f - alpha) * _speedKph;
}
