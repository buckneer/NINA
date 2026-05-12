#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

struct DashState {
    float fuelPct = 0.0f;
    float tempPct = 0.0f;
    float odoKm   = 0.0f;
    float tripKm  = 0.0f;
};

class Dashboard {
public:
    void begin(TwoWire& tempBus, TwoWire& fuelBus);
    void update(const DashState& s);
    void showText(const char* l1, const char* l2 = nullptr,
                  const char* l3 = nullptr, const char* l4 = nullptr);

    bool mainOk() const { return _mainOk; }
    bool fuelOk() const { return _fuelOk; }
    bool tempOk() const { return _tempOk; }

private:
    Adafruit_SSD1306* _fuel = nullptr;
    Adafruit_SSD1306* _temp = nullptr;
    Adafruit_SSD1306* _main = nullptr;
    bool _fuelOk = false, _tempOk = false, _mainOk = false;

    unsigned long _lastDisplay = 0;
    unsigned long _lastOdo     = 0;

    void drawBar(Adafruit_SSD1306& d, uint8_t pct);
    void renderOdometer(float km, float tripKm);
};
