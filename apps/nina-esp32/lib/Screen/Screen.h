#pragma once
#include <Adafruit_GFX.h>
#include <UINav.h>

// Abstract base for all UI screens.
// UIManager owns the display and calls clearDisplay()/display() around render().
// Screens only draw — they never clear or flush the buffer themselves.
class Screen {
public:
    virtual ~Screen() = default;

    virtual void onEnter(UINav& ui) {}
    virtual void onExit() {}
    virtual void update(UINav& ui) {}
    virtual void render(Adafruit_GFX& display) = 0;

    virtual void onNext(UINav& ui) {}
    virtual void onOk(UINav& ui) {}
    virtual void onNextLong(UINav& ui) {}

    // How often UIManager should re-render this screen (ms).
    virtual uint16_t refreshMs() const { return 200; }
};
