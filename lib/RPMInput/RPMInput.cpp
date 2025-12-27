//
// Created by Miftari Simel on 27. 12. 2025..
//

#include "RPMInput.h"

volatile uint32_t RPMInput::pulseCount = 0;

void IRAM_ATTR RPMInput::isr() {
    pulseCount++;
}

RPMInput::RPMInput(uint8_t p, uint16_t sm, uint8_t ppr)
  : pin(p), sampleMs(sm), pulsesPerRev(ppr) {}

void RPMInput::begin() {
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(pin, isr, FALLING);
    lastSampleMs = millis();
}

void RPMInput::update() {
    uint32_t now = millis();
    if (now - lastSampleMs >= sampleMs) {
        uint32_t pulses = pulseCount;
        pulseCount = 0;

        float revs = static_cast<float>(pulses) / pulsesPerRev;
        float minutes = (sampleMs / 1000.0f) / 60.0f;

        currentRPM = static_cast<uint16_t>(revs / minutes);
        lastSampleMs = now;
    }
}

uint16_t RPMInput::rpm() const {
    return currentRPM;
}