#pragma once

#include <Arduino.h>
#include <Preferences.h>

class Odometer
{
public:
    explicit Odometer(float metersPerPulse);

    void begin();

    // Pico sends its cumulative pulse count.
    void update(uint32_t pulseCount);

    void resetTrip();

    // Call before deep sleep / IGN off.
    void save();

    double km() const
    {
        return _totalKm;
    }

    double tripKm() const
    {
        return _tripKm;
    }

    uint64_t meters() const
    {
        return static_cast<uint64_t>(_totalKm * 1000.0);
    }

    uint64_t tripMeters() const
    {
        return static_cast<uint64_t>(_tripKm * 1000.0);
    }

private:
    static constexpr double INITIAL_ODO_KM = 130994.0;

    // Backup while driving in case power disappears unexpectedly.
    static constexpr double SAVE_INTERVAL_KM = 10.0;

    Preferences _preferences;

    float _metersPerPulse;

    double _totalKm = INITIAL_ODO_KM;
    double _tripKm = 0.0;

    double _lastSavedKm = INITIAL_ODO_KM;

    uint32_t _lastPulseCount = 0;
    bool _pulseInitialized = false;
};