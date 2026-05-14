#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <DashState.h>

// Manages the two 128x32 bar-graph OLEDs (fuel + temp).
// The 128x64 main OLED is owned by UIManager / DashScreen.
class Dashboard {
public:
    void begin(TwoWire& tempBus, TwoWire& fuelBus);
    void update(const DashState& s);

    bool fuelOk() const { return _fuelOk; }
    bool tempOk() const { return _tempOk; }

private:
    Adafruit_SSD1306* _fuel = nullptr;
    Adafruit_SSD1306* _temp = nullptr;
    bool _fuelOk = false, _tempOk = false;

    unsigned long _lastDisplay = 0;

    void drawBar(Adafruit_SSD1306& d, uint8_t pct);
};
