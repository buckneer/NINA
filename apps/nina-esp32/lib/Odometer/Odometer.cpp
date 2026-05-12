#include "Odometer.h"

Odometer::Odometer() {}

void Odometer::update(float speedKph) {
    const unsigned long now = millis();

    if (!_started) {
        _started = true;
    } else if (speedKph > 0.1f) {
        const float    deltaHours  = (now - _lastUpdate) / 3600000.0f;
        const uint32_t deltaMeters = static_cast<uint32_t>(speedKph * deltaHours * 1000.0f);
        _meters     += deltaMeters;
        _tripMeters += deltaMeters;
    }

    _lastUpdate = now;
}

void Odometer::resetTrip() {
    _tripMeters = 0;
}
