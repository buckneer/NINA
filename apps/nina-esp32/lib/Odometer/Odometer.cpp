#include "Odometer.h"

Odometer::Odometer(float metersPerPulse)
    : _metersPerPulse(metersPerPulse)
{
}

void Odometer::begin()
{
    _preferences.begin("odometer", false);

    // If no odometer has ever been stored, start at 130994 km.
    _totalKm = _preferences.getDouble(
        "total_km",
        INITIAL_ODO_KM);

    _lastSavedKm = _totalKm;

    // Trip intentionally resets every ESP32 restart.
    _tripKm = 0.0;

    _pulseInitialized = false;

    Serial.printf(
        "[Odometer] Total: %.3f km\n",
        _totalKm);
}

void Odometer::update(uint32_t pulseCount)
{
    // First received value only establishes our starting point.
    if (!_pulseInitialized)
    {
        _lastPulseCount = pulseCount;
        _pulseInitialized = true;
        return;
    }

    // Pico probably rebooted/reset its pulse counter.
    if (pulseCount < _lastPulseCount)
    {
        _lastPulseCount = pulseCount;
        return;
    }

    const uint32_t deltaPulses =
        pulseCount - _lastPulseCount;

    _lastPulseCount = pulseCount;

    if (deltaPulses == 0)
        return;

    const double deltaKm =
        (static_cast<double>(deltaPulses) *
         static_cast<double>(_metersPerPulse)) /
        1000.0;

    _totalKm += deltaKm;
    _tripKm += deltaKm;

    // Occasional emergency checkpoint.
    if ((_totalKm - _lastSavedKm) >= SAVE_INTERVAL_KM)
    {
        save();
    }
}

void Odometer::resetTrip()
{
    _tripKm = 0.0;
}

void Odometer::save()
{
    // Avoid unnecessary flash writes.
    if (fabs(_totalKm - _lastSavedKm) < 0.000001)
        return;

    _preferences.putDouble(
        "total_km",
        _totalKm);

    _lastSavedKm = _totalKm;

    Serial.printf(
        "[Odometer] Saved: %.3f km\n",
        _totalKm);
}