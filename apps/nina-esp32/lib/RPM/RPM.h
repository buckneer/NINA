//
// Created by Miftari Simel on 27. 12. 2025..
//

#ifndef NINA_RPM_H
#define NINA_RPM_H

#pragma once

#include <Arduino.h>
#include <Multiplex.h>

class RPMMeter
{
public:
    static constexpr uint8_t TOTAL_LEDS = 29;
    static constexpr uint8_t CHANNELS = 4;

    inline static constexpr uint8_t LEDS_PER_REG[CHANNELS] = {
        8, 8, 8, 5};

    RPMMeter(Multiplex<4> &mux, uint16_t maxRPM);

    void begin();

    // Normal RPM value from RPMInput.
    // During a sweep this value is remembered but not displayed yet.
    void setRPM(uint16_t rpm);

    // Non-blocking animation update.
    void update();

    // Start startup sweep: 0 -> max -> hold -> 0.
    void startSweep();

    bool isSweeping() const;

private:
    static void render(
        uint8_t channel,
        void *ctx,
        uint8_t *regs);

    // Actually updates the LED states.
    // This bypasses sweep ownership.
    void applyRPM(uint16_t rpm);

    bool ledStates[TOTAL_LEDS] = {false};

    Multiplex<4> &multiplex;

    uint16_t maxRPM;
    uint16_t requestedRPM = 0;

    bool sweeping = false;
    uint32_t sweepStartMs = 0;

    // Startup animation timing.
    static constexpr uint32_t SWEEP_UP_MS = 500;
    static constexpr uint32_t SWEEP_HOLD_MS = 100;
    static constexpr uint32_t SWEEP_DOWN_MS = 500;
};

#endif // NINA_RPM_H