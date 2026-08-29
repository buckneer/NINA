#include "MenuScreen.h"

void MenuScreen::addItem(MenuItem item) {
    if (_itemCount < MAX_ITEMS) {
        _items[_itemCount++] = item;
    }
}

void MenuScreen::onEnter(UINav& /*ui*/) {
    _cursor       = 0;
    _scrollOffset = 0;
}

void MenuScreen::render(Adafruit_GFX& display) {
    const uint8_t W       = display.width();
    const uint8_t H       = display.height();
    const uint8_t ROW_H   = 12;               // 8px text + 4px padding
    const uint8_t VISIBLE = H / ROW_H;
    const uint8_t TOTAL   = _itemCount + 1;   // +1 for the automatic Back entry

    // Keep cursor inside visible window
    if (_cursor < _scrollOffset)
        _scrollOffset = _cursor;
    if (_cursor >= _scrollOffset + VISIBLE)
        _scrollOffset = _cursor - VISIBLE + 1;

    for (uint8_t i = 0; i < VISIBLE; i++) {
        const uint8_t idx  = _scrollOffset + i;
        if (idx >= TOTAL) break;

        const char* label  = (idx < _itemCount) ? _items[idx].label : "Back";
        const bool  sel    = (idx == _cursor);
        const uint8_t rowY = i * ROW_H;

        if (sel) {
            display.fillRect(0, rowY, W, ROW_H, 1);
            display.setTextColor(0);
        } else {
            display.setTextColor(1);
        }

        display.setTextSize(1);
        display.setCursor(4, rowY + 2);
        display.print(label);
    }

    // Scroll indicator dot on right edge when list is longer than screen
    if (TOTAL > VISIBLE) {
        const uint8_t dotY = (_cursor * (H - 3)) / (TOTAL - 1);
        display.fillRect(W - 2, dotY, 2, 3, 1);
    }
}

void MenuScreen::onNext(UINav& /*ui*/) {
    const uint8_t TOTAL = _itemCount + 1;
    _cursor = (_cursor + 1) % TOTAL;
}

void MenuScreen::onOk(UINav& ui) {
    if (_cursor == _itemCount) {
        ui.goBack();
        return;
    }
    MenuItem& item = _items[_cursor];
    if (item.action) {
        item.action(ui);
    } else if (item.target) {
        ui.goTo(*item.target);
    }
}
