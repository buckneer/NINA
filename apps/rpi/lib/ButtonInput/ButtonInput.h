#pragma once
#include <Arduino.h>

class ButtonInput {
public:
    ButtonInput(uint8_t pin, const char* name = nullptr,
                bool activeLow = true, uint32_t debounceMs = 25);

    void begin();
    void update();

    bool pressed() const { return _stable; }

private:
    uint8_t     _pin;
    const char* _name;
    bool        _activeLow;
    uint32_t    _debounceMs;
    bool        _stable     = false;
    bool        _lastRaw    = false;
    uint32_t    _lastChange = 0;

    bool rawPressed() const;
};
