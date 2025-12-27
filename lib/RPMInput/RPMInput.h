//
// Created by Miftari Simel on 27. 12. 2025..
//

#ifndef NINA_RPMINPUT_H
#define NINA_RPMINPUT_H
#pragma once
#include <Arduino.h>

class RPMInput {
public:
    RPMInput(uint8_t pin, uint16_t sampleMs, uint8_t pulsesPerRev);

    void begin();
    void update();

    uint16_t rpm() const;

private:
    static void IRAM_ATTR isr();
    static volatile uint32_t pulseCount;

    uint8_t pin;
    uint16_t sampleMs;
    uint8_t pulsesPerRev;
    uint32_t lastSampleMs = 0;
    uint16_t currentRPM = 0;
};

#endif //NINA_RPMINPUT_H