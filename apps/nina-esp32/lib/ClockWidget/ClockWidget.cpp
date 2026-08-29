#include "ClockWidget.h"
#include <time.h>

void ClockWidget::render(Adafruit_GFX& display,
                         uint8_t x, uint8_t y,
                         uint8_t w, uint8_t h,
                         const DashState& /*s*/) {
    time_t now;
    struct tm t;
    time(&now);
    localtime_r(&now, &t);

    char timeBuf[6];   // "HH:MM"
    char dateBuf[12];  // "DD.MM.YYYY"
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d",
             t.tm_hour, t.tm_min);
    snprintf(dateBuf, sizeof(dateBuf), "%02d.%02d.%04d",
             t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);

    display.setTextColor(1);

    // Time — textSize(2): each char is 12px wide, 16px tall
    display.setTextSize(2);
    const uint8_t timeW = strlen(timeBuf) * 12;
    display.setCursor(x + (w - timeW) / 2, y + 4);
    display.print(timeBuf);

    // Date — textSize(1): each char is 6px wide, 8px tall
    display.setTextSize(1);
    const uint8_t dateW = strlen(dateBuf) * 6;
    display.setCursor(x + (w - dateW) / 2, y + 26);
    display.print(dateBuf);
}
