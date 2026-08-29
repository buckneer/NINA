#include "HallSensor.h"

HallSensor *HallSensor::_instance = nullptr;

void HallSensor::isr()
{
    if (!_instance)
        return;

    const uint32_t now = micros();
    const uint32_t last = _instance->_lastPulseUs;

    // First pulse.
    if (last == 0)
    {
        _instance->_lastPulseUs = now;
        _instance->_lastPeriodUs = 0;
        _instance->_pulseCount++;
        return;
    }

    const uint32_t period =
        now - last;

    // Reject very-close chatter/noise.
    //
    // IMPORTANT:
    // Do NOT update _lastPulseUs when rejecting.
    if (period < HALL_DEBOUNCE_US)
        return;

    // First pulse after standstill.
    if (period > HALL_STOP_TIMEOUT_US)
    {
        _instance->_lastPulseUs = now;
        _instance->_lastPeriodUs = 0;
        _instance->_pulseCount++;
        return;
    }

    // Valid pulse.
    _instance->_lastPeriodUs = period;
    _instance->_lastPulseUs = now;

    _instance->_pulseCount++;
}

HallSensor::HallSensor(
    uint8_t pin,
    float metersPerPulse,
    uint16_t sampleMs)
    : _pin(pin),
      _metersPerPulse(metersPerPulse),
      _sampleMs(sampleMs)
{
}
void HallSensor::begin()
{
    _instance = this;

    _pulseCount = 0;
    _lastPulseUs = 0;
    _lastPeriodUs = 0;

    _lastProcessedPulseCount = 0;
    _lastUpdateMs = millis();

    _speedKph = 0.0f;

    pinMode(_pin, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(_pin),
        HallSensor::isr,
        FALLING);
}

void HallSensor::update()
{
    const uint32_t nowMs = millis();

    // Don't need to process faster than this.
    if (nowMs - _lastUpdateMs < _sampleMs)
        return;

    _lastUpdateMs = nowMs;

    // Take a safe snapshot of values modified by the ISR.
    noInterrupts();

    const uint32_t pulseCount = _pulseCount;
    const uint32_t lastPulseUs = _lastPulseUs;
    const uint32_t periodUs = _lastPeriodUs;

    interrupts();

    const uint32_t nowUs = micros();

    // No pulse yet.
    if (lastPulseUs == 0)
    {
        _speedKph = 0.0f;
        return;
    }

    // No pulse for long enough -> vehicle is stopped.
    if (nowUs - lastPulseUs > HALL_STOP_TIMEOUT_US)
    {
        _speedKph = 0.0f;
        return;
    }

    // No NEW pulse since the previous update.
    //
    // Important:
    // keep the previous speed instead of setting it to zero.
    // This prevents low-speed flickering.
    if (pulseCount == _lastProcessedPulseCount)
        return;

    _lastProcessedPulseCount = pulseCount;

    // The first pulse after startup / standstill only establishes
    // a timing reference.
    if (periodUs == 0)
        return;

    // Speed:
    //
    // meters / pulse
    // ----------------  = meters / second
    // seconds / pulse
    //
    // then * 3.6 -> km/h
    const float periodSec = periodUs / 1000000.0f;

    const float rawSpeed =
        (_metersPerPulse / periodSec) * 3.6f;

    // Reject the known false high-speed signal that can occur
    // when the sensor/input oscillates while stationary.
    if (_speedKph < 5.0f && rawSpeed > 160.0f)
        return;

    // Asymmetric filtering:
    //
    // Rising speed:
    // more filtering so noise doesn't cause sudden upward jumps.
    //
    // Falling speed:
    // faster response so braking/deceleration feels natural.
    const float alpha =
        (rawSpeed < _speedKph) ? 0.50f : 0.20f;

    // If we're starting from zero, initialize directly instead
    // of slowly creeping from 0 toward the real speed.
    if (_speedKph == 0.0f)
    {
        _speedKph = rawSpeed;
    }
    else
    {
        _speedKph =
            alpha * rawSpeed +
            (1.0f - alpha) * _speedKph;
    }

    // Safety clamp.
    _speedKph = constrain(_speedKph, 0.0f, 250.0f);
}