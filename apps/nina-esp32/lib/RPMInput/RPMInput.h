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

    // ======================
    // ISR state
    // ======================

    static volatile uint32_t lastPulseUs;
    static volatile uint32_t lastGoodPeriodUs;

    static volatile uint32_t periodSumUs;
    static volatile uint16_t periodCount;

    // ======================
    // Configuration
    // ======================

    // Absolute rejection of very fast ignition ringing.
    //
    // With 2 pulses/rev:
    // 3500 us corresponds to ~8570 RPM.
    static constexpr uint32_t MIN_PULSE_US = 3500;

    // No valid pulse for 500 ms = engine stopped / signal lost.
    static constexpr uint32_t TIMEOUT_US = 500000;

    // Below roughly 1200 RPM with 2 pulses/rev,
    // allow larger real period changes for throttle response.
    static constexpr uint32_t LOW_RPM_PERIOD_US = 25000;

    // At low RPM, next pulse may arrive as early as
    // 60% of the previous valid period.
    static constexpr uint8_t LOW_RPM_MIN_PERIOD_PERCENT = 60;

    // At normal/high RPM, reject pulses arriving earlier
    // than 80% of the previous valid period.
    static constexpr uint8_t NORMAL_MIN_PERIOD_PERCENT = 80;

    // ======================
    // Output smoothing
    // ======================

    static constexpr uint16_t MOVING_THRESHOLD_RPM = 300;

    // Steady RPM: suppress small visual jitter.
    static constexpr float STEADY_ALPHA = 0.20f;

    // Engine actively changing RPM: follow more quickly.
    static constexpr float MOVING_ALPHA = 0.65f;

    // ======================
    // Instance state
    // ======================

    uint8_t pin;
    uint8_t pulsesPerRev;

    uint16_t currentRPM = 0;
};

#endif