#ifndef NINA_RPMINPUT_H
#define NINA_RPMINPUT_H

#pragma once
#include <Arduino.h>

class RPMInput
{
public:
    RPMInput(uint8_t pin, uint8_t pulsesPerRev);

    void begin();
    void update();

    uint16_t rpm() const;

private:
    static void IRAM_ATTR isr();

    static volatile uint32_t lastPulseUs;
    static volatile uint32_t periodSumUs;
    static volatile uint16_t periodCount;

    uint8_t pin;
    uint8_t pulsesPerRev;

    uint16_t currentRPM = 0;

    static constexpr uint32_t MIN_PULSE_US = 2000;
    static constexpr uint32_t TIMEOUT_US = 500000;
};

#endif