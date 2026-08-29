#pragma once
#include <Adafruit_GFX.h>
#include <DashState.h>

// Abstract interface for the swappable top-zone content on DashScreen.
// Implementations draw within the bounding box (x, y, w, h).
class DashWidget {
public:
    virtual ~DashWidget() = default;

    virtual const char* name() const = 0;
    virtual void render(Adafruit_GFX& display,
                        uint8_t x, uint8_t y,
                        uint8_t w, uint8_t h,
                        const DashState& s) = 0;
};
