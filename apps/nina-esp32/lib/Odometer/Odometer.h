#pragma once
#include <Arduino.h>

class Odometer {
public:
    Odometer();

    void update(float speedKph); // call every loop()
    void resetTrip();

    uint32_t meters()     const { return _meters; }
    uint32_t tripMeters() const { return _tripMeters; }
    float    km()         const { return _meters     / 1000.0f; }
    float    tripKm()     const { return _tripMeters / 1000.0f; }

private:
    uint32_t      _meters      = 0;
    uint32_t      _tripMeters  = 0;
    unsigned long _lastUpdate  = 0;
    bool          _started     = false;
};
