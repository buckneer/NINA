#include "HallSensor.h"

HallSensor* HallSensor::_instance = nullptr;

void HallSensor::isr() {
    if (!_instance) return;
    const uint32_t now = micros();
    if (now - _instance->_lastPulseUs >= HALL_DEBOUNCE_US) {
        _instance->_pulseCount++;
        _instance->_lastPulseUs = now;
    }
}

HallSensor::HallSensor(uint8_t pin, float metersPerPulse, uint16_t sampleMs)
    : _pin(pin), _metersPerPulse(metersPerPulse), _sampleMs(sampleMs) {}

void HallSensor::begin() {
    _instance = this;
    _lastMs   = millis();
    pinMode(_pin, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(_pin), HallSensor::isr, RISING);
}

void HallSensor::update() {
    const uint32_t now = millis();
    if (now - _lastMs < _sampleMs) return;

    const uint32_t pulses = _pulseCount - _lastSnap;
    const float    dtSec  = (now - _lastMs) / 1000.0f;
    _speedKph  = (pulses * _metersPerPulse / dtSec) * 3.6f;
    _lastSnap  = _pulseCount;
    _lastMs    = now;
}
