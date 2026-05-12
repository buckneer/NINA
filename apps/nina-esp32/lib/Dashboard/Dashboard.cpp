#include "Dashboard.h"

static const unsigned char PROGMEM bmp_cap[]    = { 0xf8, 0x20, 0xf8 };
static const unsigned char PROGMEM bmp_tray[]   = {
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,
    0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x00,0x00,0x00,0x00,
    0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x00,
    0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x01
};
static const unsigned char PROGMEM bmp_base[]   = { 0x70, 0x88, 0x50 };

void Dashboard::begin(TwoWire& tempBus, TwoWire& fuelBus) {
    _fuel = new Adafruit_SSD1306(128, 32, &fuelBus, -1);
    _temp = new Adafruit_SSD1306(128, 32, &tempBus, -1);
    _main = new Adafruit_SSD1306(128, 64, &fuelBus, -1);

    _fuelOk = _fuel->begin(SSD1306_SWITCHCAPVCC, 0x3C);
    _tempOk = _temp->begin(SSD1306_SWITCHCAPVCC, 0x3C);
    _mainOk = _main->begin(SSD1306_SWITCHCAPVCC, 0x3D);

    if (!_fuelOk) { delete _fuel; _fuel = nullptr; }
    if (!_tempOk) { delete _temp; _temp = nullptr; }
    if (!_mainOk) { delete _main; _main = nullptr; }

    Serial.printf("[Dashboard] fuel=%d  temp=%d  main=%d\n", _fuelOk, _tempOk, _mainOk);

    if (_fuelOk) { _fuel->clearDisplay(); _fuel->display(); }
    if (_tempOk) { _temp->clearDisplay(); _temp->display(); }
    if (_mainOk) { _main->clearDisplay(); _main->display(); renderOdometer(0, 0); }
}

void Dashboard::update(const DashState& s) {
    const unsigned long now = millis();

    if (now - _lastDisplay >= 200) {
        _lastDisplay = now;
        if (_fuelOk && _fuel) drawBar(*_fuel, static_cast<uint8_t>(s.fuelPct));
        if (_tempOk && _temp) drawBar(*_temp, static_cast<uint8_t>(s.tempPct));
    }

    if (now - _lastOdo >= 500) {
        _lastOdo = now;
        if (_mainOk && _main) renderOdometer(s.odoKm, s.tripKm);
    }
}

void Dashboard::showText(const char* l1, const char* l2, const char* l3, const char* l4) {
    if (!_mainOk || !_main) return;
    _main->clearDisplay();
    _main->setTextSize(1);
    _main->setTextColor(SSD1306_WHITE);
    const char* lines[] = { l1, l2, l3, l4 };
    for (uint8_t i = 0; i < 4; i++) {
        if (!lines[i]) break;
        _main->setCursor(0, i * 16);
        _main->println(lines[i]);
    }
    _main->display();
}

void Dashboard::drawBar(Adafruit_SSD1306& d, uint8_t pct) {
    d.clearDisplay();
    d.drawRoundRect(1, 1, 126, 22, 3, SSD1306_WHITE);
    const uint8_t bars = (pct * 10) / 100;
    for (uint8_t i = 0; i < bars; i++)
        d.fillRect(5 + 12 * i, 4, 10, 16, SSD1306_WHITE);
    d.drawBitmap(121, 29, bmp_cap,  5,   3, SSD1306_WHITE);
    d.drawBitmap(  4, 24, bmp_tray, 120, 4, SSD1306_WHITE);
    d.drawBitmap(  2, 29, bmp_base, 5,   3, SSD1306_WHITE);
    d.display();
}

void Dashboard::renderOdometer(float km, float tripKm) {
    if (!_main) return;
    const uint32_t odo  = static_cast<uint32_t>(km);
    const uint32_t trip = static_cast<uint32_t>(tripKm);

    char odoStr[16], tripStr[16];
    if (odo >= 1000) snprintf(odoStr,  sizeof(odoStr),  "%lu,%03lu", odo  / 1000, odo  % 1000);
    else             snprintf(odoStr,  sizeof(odoStr),  "%lu", odo);
    if (trip >= 1000) snprintf(tripStr, sizeof(tripStr), "%lu,%03lu", trip / 1000, trip % 1000);
    else              snprintf(tripStr, sizeof(tripStr), "%lu", trip);

    _main->clearDisplay();
    _main->setTextColor(SSD1306_WHITE);
    _main->setTextWrap(false);

    _main->setTextSize(2);
    _main->setCursor(35, 5);
    _main->print("--:--");

    _main->setTextSize(1);
    _main->setCursor(34, 24);
    _main->print("--.--.----");

    _main->setCursor(3,  44); _main->print("Trip: ");
    _main->setCursor(60, 44); _main->print(tripStr);
    _main->drawLine(0, 53, 128, 53, SSD1306_WHITE);
    _main->setCursor(60, 55); _main->print(odoStr);

    _main->display();
}
