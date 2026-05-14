#include "Dashboard.h"

static const unsigned char PROGMEM bmp_cap[]  = { 0xf8, 0x20, 0xf8 };
static const unsigned char PROGMEM bmp_tray[] = {
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,
    0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x00,0x00,0x00,0x00,
    0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x00,
    0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x01
};
static const unsigned char PROGMEM bmp_base[] = { 0x70, 0x88, 0x50 };

void Dashboard::begin(TwoWire& tempBus, TwoWire& fuelBus) {
    _fuel = new Adafruit_SSD1306(128, 32, &fuelBus, -1);
    _temp = new Adafruit_SSD1306(128, 32, &tempBus, -1);

    _fuelOk = _fuel->begin(SSD1306_SWITCHCAPVCC, 0x3C);
    _tempOk = _temp->begin(SSD1306_SWITCHCAPVCC, 0x3C);

    if (!_fuelOk) { delete _fuel; _fuel = nullptr; }
    if (!_tempOk) { delete _temp; _temp = nullptr; }

    Serial.printf("[Dashboard] fuel=%d  temp=%d\n", _fuelOk, _tempOk);

    if (_fuelOk) { _fuel->clearDisplay(); _fuel->display(); }
    if (_tempOk) { _temp->clearDisplay(); _temp->display(); }
}

void Dashboard::update(const DashState& s) {
    const unsigned long now = millis();
    if (now - _lastDisplay < 200) return;
    _lastDisplay = now;

    if (_fuelOk && _fuel) drawBar(*_fuel, static_cast<uint8_t>(s.fuelPct));
    if (_tempOk && _temp) drawBar(*_temp, static_cast<uint8_t>(s.tempPct));
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
