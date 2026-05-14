#pragma once
#include <Arduino.h>
#include <Wire.h>

class PicoLink {
public:
    explicit PicoLink(uint8_t addr = 0x42);

    void begin(TwoWire& bus0, TwoWire& bus1); // auto-detects which bus the Pico is on
    void begin(TwoWire& wire);                // use a specific bus directly

    void update(); // call every loop()

    // Raw state
    float    speedKph()    const { return _speedKph; }
    uint8_t  buttons()     const { return _buttons; }
    uint32_t nextHeldMs()  const { return _nextHeldMs; }
    uint32_t okHeldMs()    const { return _okHeldMs; }

    // Short press – edge-consuming, fires once per press
    bool nextPressed();
    bool okPressed();

    // Long press – edge-consuming, fires once when hold crosses threshold
    bool nextLongPressed(uint32_t thresholdMs = 800);
    bool okLongPressed(uint32_t thresholdMs = 800);

    // Accel tick – fires repeatedly while held, starting slow and speeding up
    // startMs   : hold duration before first tick (default = 800 ms)
    // slowMs    : initial tick interval in ms     (default = 400 ms)
    // fastMs    : minimum tick interval in ms     (default =  80 ms)
    bool nextAccelTick(uint32_t startMs = 800, uint32_t slowMs = 400, uint32_t fastMs = 80);
    bool okAccelTick  (uint32_t startMs = 800, uint32_t slowMs = 400, uint32_t fastMs = 80);

private:
    static bool scanForAddr(TwoWire& bus, const char* label, uint8_t addr);
    bool        accelTick(uint32_t heldMs, uint32_t startMs, uint32_t slowMs,
                          uint32_t fastMs, unsigned long& lastTick);

    TwoWire*      _wire          = nullptr;
    uint8_t       _addr;

    float         _speedKph      = 0.0f;
    uint8_t       _buttons       = 0;
    uint8_t       _lastButtons   = 0;
    uint8_t       _pressedEdges  = 0;
    uint32_t      _nextHeldMs    = 0;
    uint32_t      _okHeldMs      = 0;

    bool          _nextLongFired = false;
    bool          _okLongFired   = false;
    unsigned long _nextLastTick  = 0;
    unsigned long _okLastTick    = 0;

    unsigned long _lastPoll      = 0;
    unsigned long _lastLog       = 0;

    static constexpr uint8_t  PACKET_LEN = 7;
    static constexpr uint32_t POLL_MS    = 50;
    static constexpr uint32_t LOG_MS     = 1000;
};
