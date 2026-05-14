#pragma once
#include <DashWidget.h>

// Top-zone widget showing the current time and date.
// Time is read from the ESP32 internal RTC via time().
// Use ClockSetScreen (from the settings menu) to set it.
class ClockWidget : public DashWidget {
public:
    const char* name() const override { return "Clock"; }

    void render(Adafruit_GFX& display,
                uint8_t x, uint8_t y,
                uint8_t w, uint8_t h,
                const DashState& s) override;
};
