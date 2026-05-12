#include "ButtonInput.h"

ButtonInput::ButtonInput(uint8_t pin, const char* name, bool activeLow, uint32_t debounceMs)
    : _pin(pin), _name(name), _activeLow(activeLow), _debounceMs(debounceMs) {}

bool ButtonInput::rawPressed() const {
    return _activeLow ? (digitalRead(_pin) == LOW) : (digitalRead(_pin) == HIGH);
}

void ButtonInput::begin() {
    pinMode(_pin, INPUT_PULLUP);
    _stable     = rawPressed();
    _lastRaw    = _stable;
    _lastChange = millis();
}

void ButtonInput::update() {
    const bool     raw = rawPressed();
    const uint32_t now = millis();

    if (raw != _lastRaw) {
        _lastRaw    = raw;
        _lastChange = now;
    }

    if ((now - _lastChange) >= _debounceMs && raw != _stable) {
        _stable = raw;
        if (_name)
            printf("[BTN] %s %s\n", _name, _stable ? "PRESSED" : "released");
    }
}
