#pragma once
#include <Arduino.h>

class ButtonInput {
public:
    ButtonInput(uint8_t pin, const char* name = nullptr,
                bool activeLow = true, uint32_t debounceMs = 25);

    void begin();
    void update();

    bool     pressed()  const { return _stable; }
    uint32_t holdMs()   const; // ms since stable press started (0 if not pressed)
    bool     heldFor(uint32_t ms) const;   // true while held >= ms
    bool     longPressed(uint32_t ms = 800); // true once when threshold first crossed (edge-consuming)

private:
    uint8_t     _pin;
    const char* _name;
    bool        _activeLow;
    uint32_t    _debounceMs;
    bool        _stable        = false;
    bool        _lastRaw       = false;
    uint32_t    _lastChange    = 0;  // last debounce-settle timestamp
    uint32_t    _pressedSince  = 0;  // when the stable press started
    bool        _longFired     = false;

    bool rawPressed() const;
};
