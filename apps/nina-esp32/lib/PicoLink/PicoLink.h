#pragma once
#include <Arduino.h>
#include <Wire.h>

class PicoLink {
public:
    explicit PicoLink(uint8_t addr = 0x42);

    void begin(TwoWire& bus0, TwoWire& bus1); // auto-detects which bus the Pico is on
    void begin(TwoWire& wire);                // use a specific bus directly

    void update(); // call every loop()

    float   speedKph() const { return _speedKph; }
    uint8_t buttons()  const { return _buttons; }

    bool nextPressed(); // true once per press (edge-consuming)
    bool okPressed();   // true once per press (edge-consuming)

private:
    static bool scanForAddr(TwoWire& bus, const char* label, uint8_t addr);

    TwoWire*      _wire         = nullptr;
    uint8_t       _addr;
    float         _speedKph     = 0.0f;
    uint8_t       _buttons      = 0;
    uint8_t       _lastButtons  = 0;
    uint8_t       _pressedEdges = 0;
    unsigned long _lastPoll     = 0;
    unsigned long _lastLog      = 0;

    static constexpr uint8_t  PACKET_LEN = 5;
    static constexpr uint32_t POLL_MS    = 50;
    static constexpr uint32_t LOG_MS     = 1000;
};
