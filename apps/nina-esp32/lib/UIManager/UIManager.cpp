#include "UIManager.h"
#include <Adafruit_SSD1306.h>
#include <PicoLink.h>
#include <MenuScreen.h>
#include "YugoLogo.h"

// ─── setup ───────────────────────────────────────────────────────────────────

void UIManager::begin(TwoWire& bus, PicoLink& pico, Screen& defaultScreen) {
    _pico    = &pico;
    _default = &defaultScreen;

    _oled = new Adafruit_SSD1306(128, 64, &bus, -1);

    if (!_oled->begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
        Serial.println("[UIManager] main OLED init failed (addr 0x3D)");
        delete _oled;
        _oled = nullptr;
        return;
    }

    _oled->clearDisplay();
    _oled->drawBitmap(32, 0, yugo_logo_white, 128, 64, WHITE);
    _oled->display();
    delay(2000);
    _oled->clearDisplay();
    _oled->display();

    _default->onEnter(*this);
    Serial.println("[UIManager] ready");
}

void UIManager::setMenu(MenuScreen& menu) {
    _menu = &menu;
}

// ─── navigation ──────────────────────────────────────────────────────────────

Screen* UIManager::active() {
    return (_stackTop >= 0) ? _stack[_stackTop] : _default;
}

void UIManager::goTo(Screen& screen) {
    if (_stackTop >= MAX_STACK - 1) return;
    active()->onExit();
    _stack[++_stackTop] = &screen;
    screen.onEnter(*this);
    _lastRender = 0;
}

void UIManager::goBack() {
    if (_stackTop < 0) return;
    active()->onExit();
    _stackTop--;
    active()->onEnter(*this);
    _lastRender = 0;
}

void UIManager::goToDefault() {
    if (_stackTop < 0) return;
    active()->onExit();
    _stackTop = -1;
    _default->onEnter(*this);
    _lastRender = 0;
}

// ─── update loop ─────────────────────────────────────────────────────────────

void UIManager::update() {
    if (!_oled) return;

    // Long OK always jumps to the settings menu from any screen.
    // Any in-progress sub-screen is abandoned.
    if (_menu && _pico->okLongPressed(MENU_HOLD_MS)) {
        if (active() != static_cast<Screen*>(_menu)) {
            active()->onExit();
            _stackTop    = -1;
            _stack[++_stackTop] = _menu;
            _menu->onEnter(*this);
            _lastRender  = 0;
        }
        return;
    }

    if (_pico->nextPressed())         active()->onNext(*this);
    if (_pico->okPressed())           active()->onOk(*this);
    if (_pico->nextLongPressed(800))  active()->onNextLong(*this);

    active()->update(*this);

    renderFrame();
}

void UIManager::renderFrame() {
    const unsigned long now = millis();
    if (now - _lastRender < active()->refreshMs()) return;
    _lastRender = now;

    _oled->clearDisplay();
    active()->render(*_oled);
    _oled->display();
}
