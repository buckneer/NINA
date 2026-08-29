#include "RPMInput.h"

// ======================
// ISR state
// ======================

volatile uint32_t RPMInput::lastPulseUs = 0;
volatile uint32_t RPMInput::periodSumUs = 0;
volatile uint16_t RPMInput::periodCount = 0;

// ======================
// Implementation
// ======================

RPMInput::RPMInput(uint8_t p, uint8_t ppr)
    : pin(p),
      pulsesPerRev(ppr)
{
}

void IRAM_ATTR RPMInput::isr()
{
    const uint32_t now = micros();

    // First edge establishes timing reference.
    if (lastPulseUs == 0)
    {
        lastPulseUs = now;
        return;
    }

    const uint32_t period = now - lastPulseUs;

    // Reject extra edges caused by ignition ringing/noise.
    //
    // Important:
    // Do NOT update lastPulseUs for rejected pulses.
    if (period < MIN_PULSE_US)
    {
        return;
    }

    // Signal disappeared for a long time.
    if (period > TIMEOUT_US)
    {
        lastPulseUs = now;
        return;
    }

    // Valid pulse.
    lastPulseUs = now;

    periodSumUs += period;
    periodCount++;
}

void RPMInput::begin()
{
    pinMode(pin, INPUT_PULLUP);

    noInterrupts();

    lastPulseUs = 0;
    periodSumUs = 0;
    periodCount = 0;

    interrupts();

    currentRPM = 0;
    pendingRPM = 0;
    pendingDirection = 0;

    attachInterrupt(
        digitalPinToInterrupt(pin),
        isr,
        FALLING);
}

void RPMInput::update()
{
    uint32_t sum;
    uint16_t count;
    uint32_t lastPulse;

    // Snapshot ISR state.
    noInterrupts();

    sum = periodSumUs;
    count = periodCount;
    lastPulse = lastPulseUs;

    periodSumUs = 0;
    periodCount = 0;

    interrupts();

    const uint32_t now = micros();

    // Engine stopped / RPM signal disappeared.
    if (lastPulse == 0 || (now - lastPulse) > TIMEOUT_US)
    {
        currentRPM = 0;
        pendingRPM = 0;
        pendingDirection = 0;
        return;
    }

    if (count == 0)
    {
        return;
    }

    const float averagePeriodUs =
        static_cast<float>(sum) /
        static_cast<float>(count);

    if (averagePeriodUs <= 0.0f)
    {
        return;
    }

    const float calculatedRPM =
        60000000.0f /
        (averagePeriodUs *
         static_cast<float>(pulsesPerRev));

    const uint16_t newRPM =
        static_cast<uint16_t>(calculatedRPM + 0.5f);

    // First valid measurement.
    if (currentRPM == 0)
    {
        currentRPM = newRPM;
        pendingRPM = 0;
        pendingDirection = 0;
        return;
    }

    // =========================
    // Plausibility filtering
    // =========================

    const int32_t delta =
        static_cast<int32_t>(newRPM) -
        static_cast<int32_t>(currentRPM);

    const uint16_t maxRise =
        max<uint16_t>(
            MIN_RISE_ALLOWANCE,
            currentRPM / 2);

    const uint16_t maxFall =
        max<uint16_t>(
            MIN_FALL_ALLOWANCE,
            currentRPM / 3);

    const bool largeRise =
        delta > static_cast<int32_t>(maxRise);

    const bool largeFall =
        delta < -static_cast<int32_t>(maxFall);

    // =========================
    // Large-jump confirmation
    // =========================

    if (largeRise || largeFall)
    {
        const int8_t direction =
            (delta > 0) ? 1 : -1;

        // First suspicious large jump.
        if (pendingDirection != direction)
        {
            pendingRPM = newRPM;
            pendingDirection = direction;
            return;
        }

        // Second jump in the same direction.
        const int32_t pendingDelta =
            static_cast<int32_t>(newRPM) -
            static_cast<int32_t>(pendingRPM);

        const uint32_t confirmWindow =
            max<uint32_t>(
                400,
                pendingRPM / 3);

        if (
            abs(pendingDelta) <=
            static_cast<int32_t>(confirmWindow))
        {
            // Confirmed genuine fast RPM change.
            currentRPM =
                static_cast<uint16_t>(
                    CONFIRMED_JUMP_ALPHA *
                        static_cast<float>(newRPM) +
                    (1.0f - CONFIRMED_JUMP_ALPHA) *
                        static_cast<float>(currentRPM) +
                    0.5f);

            pendingRPM = 0;
            pendingDirection = 0;
            return;
        }

        // Still inconsistent; wait for another confirmation.
        pendingRPM = newRPM;
        pendingDirection = direction;
        return;
    }

    // A normal sample cancels a pending suspicious jump.
    pendingRPM = 0;
    pendingDirection = 0;

    // =========================
    // Adaptive smoothing
    // =========================

    const uint32_t rpmDiff =
        static_cast<uint32_t>(
            abs(
                static_cast<int32_t>(newRPM) -
                static_cast<int32_t>(currentRPM)));

    const float alpha =
        (rpmDiff > MOVING_THRESHOLD_RPM)
            ? MOVING_ALPHA
            : NORMAL_ALPHA;

    currentRPM =
        static_cast<uint16_t>(
            alpha * static_cast<float>(newRPM) +
            (1.0f - alpha) *
                static_cast<float>(currentRPM) +
            0.5f);
}

uint16_t RPMInput::rpm() const
{
    return currentRPM;
}