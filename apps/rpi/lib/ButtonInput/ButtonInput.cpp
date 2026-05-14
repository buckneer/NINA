#include "ButtonInput.h"

ButtonInput::ButtonInput(uint8_t pin, const char* name, bool activeLow, uint32_t debounceMs)
    : _pin(pin), _name(name), _activeLow(activeLow), _debounceMs(debounceMs) {}

bool ButtonInput::rawPressed() const {
    return _activeLow ? (digitalRead(_pin) == LOW) : (digitalRead(_pin) == HIGH);
}

void ButtonInput::begin() {
    pinMode(_pin, INPUT_PULLUP);
    _stable       = rawPressed();
    _lastRaw      = _stable;
    _lastChange   = millis();
    _pressedSince = _stable ? millis() : 0;
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
        if (_stable) {
            _pressedSince = now;
            _longFired    = false;
            if (_name) printf("[BTN] %s PRESSED\n", _name);
        } else {
            _pressedSince = 0;
            _longFired    = false;
            if (_name) printf("[BTN] %s released\n", _name);
        }
    }
}

uint32_t ButtonInput::holdMs() const {
    return (_stable && _pressedSince > 0) ? (millis() - _pressedSince) : 0;
}

bool ButtonInput::heldFor(uint32_t ms) const {
    return _stable && _pressedSince > 0 && (millis() - _pressedSince) >= ms;
}

bool ButtonInput::longPressed(uint32_t ms) {
    if (!_longFired && heldFor(ms)) {
        _longFired = true;
        if (_name) printf("[BTN] %s LONG PRESS\n", _name);
        return true;
    }
    return false;
}
