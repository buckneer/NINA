#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <UINav.h>
#include <Screen.h>

// Forward-declared so UIManager.h can be included by main.cpp without
// pulling display driver or PicoLink headers into every translation unit.
// The driver used is Adafruit_SSD1306 (confirmed working with this display).
// If a true SSD1309 init sequence is ever needed, swap the forward declaration
// and the include/ctor in UIManager.cpp only — nothing else changes.
class Adafruit_SSD1306;
class PicoLink;
class MenuScreen;

// Central UI orchestrator.
//
// Owns the 128x64 SSD1309 main OLED (0x3D on the fuel I2C bus).
// Reads button events from PicoLink and routes them to the active Screen.
// Manages a shallow navigation stack (max depth 3) so sub-screens can
// return to their caller via goBack().
//
// Long-press OK (>=1000 ms) always opens the settings menu, from any screen.
//
// Note: pico.update() must be called by the main loop BEFORE ui.update().
//       UIManager only reads button edges — it does not poll the Pico.
class UIManager : public UINav {
public:
    void begin(TwoWire& bus, PicoLink& pico, Screen& defaultScreen);
    void setMenu(MenuScreen& menu);

    void update();

    // UINav interface
    void goTo(Screen& screen) override;
    void goBack() override;
    void goToDefault() override;
    PicoLink& pico() override { return *_pico; }

private:
    static constexpr uint8_t  MAX_STACK    = 3;
    static constexpr uint32_t MENU_HOLD_MS = 1000;

    Adafruit_SSD1306*      _oled       = nullptr;
    PicoLink*              _pico       = nullptr;
    Screen*                _default    = nullptr;
    MenuScreen*            _menu       = nullptr;

    Screen*       _stack[MAX_STACK] = {};
    int8_t        _stackTop         = -1;
    unsigned long _lastRender       = 0;

    Screen* active();
    void    renderFrame();
};
