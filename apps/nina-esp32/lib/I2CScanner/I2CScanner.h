#pragma once
#include <Arduino.h>
#include <Wire.h>

namespace I2CScanner {
    inline void scan(TwoWire& bus, const char* label) {
        Serial.printf("[I2C] %s scanning...\n", label);
        uint8_t count = 0;
        for (uint8_t addr = 1; addr < 127; addr++) {
            bus.beginTransmission(addr);
            if (bus.endTransmission() == 0) {
                Serial.printf("[I2C] %s: 0x%02X\n", label, addr);
                count++;
            }
            delay(2);
        }
        if (count == 0) Serial.printf("[I2C] %s: no devices\n", label);
        else            Serial.printf("[I2C] %s: %u device(s)\n", label, count);
    }
}
