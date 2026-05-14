#pragma once

// Minimal navigation interface exposed to Screen subclasses.
// Screens only depend on this header — no display drivers, no PicoLink.
// UIManager implements this interface.

class Screen;
class PicoLink;

class UINav {
public:
    virtual ~UINav() = default;

    virtual void goTo(Screen& s) = 0;
    virtual void goBack() = 0;
    virtual void goToDefault() = 0;

    // Access the raw PicoLink for screens that need accel-tick or hold info.
    // Callers must #include <PicoLink.h> to use the returned reference.
    virtual PicoLink& pico() = 0;
};
