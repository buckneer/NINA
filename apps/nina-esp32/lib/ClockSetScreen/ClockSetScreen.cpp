#include "ClockSetScreen.h"
#include <PicoLink.h>
#include <sys/time.h>
#include <time.h>

// ─── lifecycle ───────────────────────────────────────────────────────────────

void ClockSetScreen::onEnter(UINav& /*ui*/) {
    _field = HOUR;
    loadCurrentTime();
}

void ClockSetScreen::loadCurrentTime() {
    time_t now;
    struct tm t;
    time(&now);
    localtime_r(&now, &t);
    _hour   = t.tm_hour;
    _minute = t.tm_min;
    _day    = t.tm_mday;
    _month  = t.tm_mon + 1;
    _year   = t.tm_year + 1900;
}

// ─── input ───────────────────────────────────────────────────────────────────

void ClockSetScreen::update(UINav& ui) {
    // Accel tick fires repeatedly while NEXT is held, speeding up over time.
    if (ui.pico().nextAccelTick()) {
        increment();
    }
}

void ClockSetScreen::onNext(UINav& /*ui*/) {
    increment();
}

void ClockSetScreen::onOk(UINav& ui) {
    if (_field == YEAR) {
        saveAndExit(ui);
    } else {
        _field = static_cast<Field>(_field + 1);
    }
}

void ClockSetScreen::increment() {
    int16_t v = value() + 1;
    if (v > maxVal()) v = minVal();
    setValue(v);
}

void ClockSetScreen::saveAndExit(UINav& ui) {
    struct tm t  = {};
    t.tm_hour    = _hour;
    t.tm_min     = _minute;
    t.tm_sec     = 0;
    t.tm_mday    = _day;
    t.tm_mon     = _month - 1;
    t.tm_year    = _year - 1900;
    t.tm_isdst   = -1;

    time_t epoch = mktime(&t);
    struct timeval tv;
    tv.tv_sec  = epoch;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);

    ui.goBack();
}

// ─── render ──────────────────────────────────────────────────────────────────

void ClockSetScreen::render(Adafruit_GFX& display) {
    const uint8_t W = display.width();
    const uint8_t H = display.height();

    // Header: "Set Hour" etc.
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(2, 2);
    display.print("Set ");
    display.print(fieldName());

    display.drawFastHLine(0, 13, W, 1);

    // Large value in centre
    char buf[8];
    if (_field == YEAR)
        snprintf(buf, sizeof(buf), "%d", static_cast<int>(value()));
    else
        snprintf(buf, sizeof(buf), "%02d", static_cast<int>(value()));

    display.setTextSize(3);   // 18px wide, 24px tall per char
    const uint8_t tw = strlen(buf) * 18;
    display.setCursor((W - tw) / 2, 20);
    display.print(buf);

    // Bottom hint
    display.drawFastHLine(0, H - 12, W, 1);
    display.setTextSize(1);
    display.setCursor(2, H - 9);
    display.print("NEXT:+  OK:");
    display.print(_field == YEAR ? "save" : "next");
}

// ─── field helpers ────────────────────────────────────────────────────────────

int16_t ClockSetScreen::value() const {
    switch (_field) {
        case HOUR:   return _hour;
        case MINUTE: return _minute;
        case DAY:    return _day;
        case MONTH:  return _month;
        case YEAR:   return _year;
    }
    return 0;
}

void ClockSetScreen::setValue(int16_t v) {
    switch (_field) {
        case HOUR:   _hour   = v; break;
        case MINUTE: _minute = v; break;
        case DAY:    _day    = v; break;
        case MONTH:  _month  = v; break;
        case YEAR:   _year   = v; break;
    }
}

int16_t ClockSetScreen::minVal() const {
    switch (_field) {
        case HOUR:   return 0;
        case MINUTE: return 0;
        case DAY:    return 1;
        case MONTH:  return 1;
        case YEAR:   return 2020;
    }
    return 0;
}

int16_t ClockSetScreen::maxVal() const {
    switch (_field) {
        case HOUR:   return 23;
        case MINUTE: return 59;
        case DAY:    return 31;
        case MONTH:  return 12;
        case YEAR:   return 2099;
    }
    return 0;
}

const char* ClockSetScreen::fieldName() const {
    switch (_field) {
        case HOUR:   return "Hour";
        case MINUTE: return "Minute";
        case DAY:    return "Day";
        case MONTH:  return "Month";
        case YEAR:   return "Year";
    }
    return "";
}
