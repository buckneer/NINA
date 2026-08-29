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

    // Large-jump confirmation.
    uint16_t pendingRPM = 0;
    int8_t pendingDirection = 0;

    // ============================
    // Input / noise configuration
    // ============================

    // Software protection against ignition ringing.
    //
    // Once LM393 hysteresis is added, we may be able to reduce this
    // slightly if faster response is desired.
    static constexpr uint32_t MIN_PULSE_US = 3500;

    // No valid pulse for this long = engine stopped / signal lost.
    static constexpr uint32_t TIMEOUT_US = 500000;

    // ============================
    // RPM filtering configuration
    // ============================

    static constexpr uint16_t MIN_RISE_ALLOWANCE = 800;
    static constexpr uint16_t MIN_FALL_ALLOWANCE = 500;

    static constexpr float NORMAL_ALPHA = 0.20f;
    static constexpr float MOVING_ALPHA = 0.55f;
    static constexpr float CONFIRMED_JUMP_ALPHA = 0.75f;

    static constexpr uint16_t MOVING_THRESHOLD_RPM = 500;
};

#endif