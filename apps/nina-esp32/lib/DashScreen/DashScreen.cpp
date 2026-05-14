#include "DashScreen.h"

static constexpr uint8_t STRIP_H = 22;  // bottom strip height in pixels

void DashScreen::render(Adafruit_GFX& display) {
    const uint8_t W     = display.width();
    const uint8_t H     = display.height();
    const uint8_t TOP_H = H - STRIP_H;

    if (_topWidget) {
        _topWidget->render(display, 0, 0, W, TOP_H, _state);
    }

    renderBottomStrip(display);
}

void DashScreen::renderBottomStrip(Adafruit_GFX& display) {
    const uint8_t W  = display.width();
    const uint8_t H  = display.height();
    const uint8_t y0 = H - STRIP_H;

    display.drawFastHLine(0, y0, W, 1);

    char tripStr[20];
    const uint32_t trip = static_cast<uint32_t>(_state.tripKm);
    if (trip >= 1000)
        snprintf(tripStr, sizeof(tripStr), "Trip: %lu,%03lu km", trip / 1000, trip % 1000);
    else
        snprintf(tripStr, sizeof(tripStr), "Trip: %lu km", trip);

    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(2, y0 + 2);
    display.print(tripStr);

    display.drawFastHLine(0, y0 + 11, W, 1);

    char odoStr[20];
    const uint32_t odo = static_cast<uint32_t>(_state.odoKm);
    if (odo >= 1000)
        snprintf(odoStr, sizeof(odoStr), "Odo:  %lu,%03lu km", odo / 1000, odo % 1000);
    else
        snprintf(odoStr, sizeof(odoStr), "Odo:  %lu km", odo);

    display.setCursor(2, y0 + 13);
    display.print(odoStr);
}
