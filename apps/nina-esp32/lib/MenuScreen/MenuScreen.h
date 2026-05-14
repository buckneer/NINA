#pragma once
#include <Screen.h>   // also pulls in UINav.h
#include <Arduino.h>

struct MenuItem {
    const char* label  = nullptr;
    void (*action)(UINav& ui) = nullptr;
    Screen*     target = nullptr;

    MenuItem() = default;
    MenuItem(const char* l,
             void (*a)(UINav&) = nullptr,
             Screen* t         = nullptr)
        : label(l), action(a), target(t) {}
};

// Scrollable menu screen.
// Items are registered with addItem(). A "Back" entry is always appended
// automatically and calls ui.goBack() when selected.
//
// NEXT scrolls the cursor down (wraps). OK executes action or navigates.
// Selected row is rendered inverted (white background, black text).
class MenuScreen : public Screen {
public:
    static constexpr uint8_t MAX_ITEMS = 10;

    void addItem(MenuItem item);

    void onEnter(UINav& ui) override;
    void render(Adafruit_GFX& display) override;
    void onNext(UINav& ui) override;
    void onOk(UINav& ui) override;

    uint16_t refreshMs() const override { return 80; }

private:
    MenuItem _items[MAX_ITEMS];
    uint8_t  _itemCount    = 0;
    uint8_t  _cursor       = 0;
    uint8_t  _scrollOffset = 0;
};
