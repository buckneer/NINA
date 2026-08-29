#include "RPMInput.h"

// ======================
// ISR state
// ======================

volatile uint32_t RPMInput::lastPulseUs = 0;
volatile uint32_t RPMInput::lastGoodPeriodUs = 0;

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

// ============================================================================
// Interrupt
// ============================================================================

void IRAM_ATTR RPMInput::isr()
{
    const uint32_t now = micros();

    // First edge only establishes our reference.
    if (lastPulseUs == 0)
    {
        lastPulseUs = now;
        return;
    }

    const uint32_t period =
        now - lastPulseUs;

    // ------------------------------------------------------------------------
    // Absolute noise rejection
    // ------------------------------------------------------------------------
    //
    // Any pulse arriving this quickly is impossible for the engine.
    //
    // IMPORTANT:
    // Do NOT update lastPulseUs.
    //
    // If this is ignition ringing, the next genuine pulse must still be
    // measured from the previous genuine pulse.
    //

    if (period < MIN_PULSE_US)
    {
        return;
    }

    // ------------------------------------------------------------------------
    // Signal timeout
    // ------------------------------------------------------------------------

    if (period > TIMEOUT_US)
    {
        // Engine stopped / signal disappeared.
        // Start a fresh sequence.
        lastPulseUs = now;
        lastGoodPeriodUs = 0;

        return;
    }

    // ------------------------------------------------------------------------
    // Adaptive early-pulse rejection
    // ------------------------------------------------------------------------
    //
    // The engine cannot suddenly shorten its ignition period dramatically
    // between two consecutive ignition events.
    //
    // Example:
    //
    // previous valid period = 15 ms
    //
    // At normal RPM:
    // minimum accepted next period = 15 ms * 80% = 12 ms
    //
    // A noise edge at 8 ms is therefore rejected.
    //

    if (lastGoodPeriodUs != 0)
    {
        uint8_t minimumPercent;

        if (lastGoodPeriodUs > LOW_RPM_PERIOD_US)
        {
            // At idle / low RPM, allow faster genuine acceleration.
            minimumPercent =
                LOW_RPM_MIN_PERIOD_PERCENT;
        }
        else
        {
            // At normal/high RPM, be more aggressive against
            // extra ignition edges.
            minimumPercent =
                NORMAL_MIN_PERIOD_PERCENT;
        }

        const uint32_t dynamicMinimumPeriod =
            (lastGoodPeriodUs *
             static_cast<uint32_t>(minimumPercent)) /
            100UL;

        if (period < dynamicMinimumPeriod)
        {
            // Suspicious early edge.
            //
            // DO NOT update lastPulseUs.
            //
            // The next genuine ignition edge will therefore be measured
            // from the previous genuine edge.
            return;
        }
    }

    // ------------------------------------------------------------------------
    // Valid pulse
    // ------------------------------------------------------------------------

    lastPulseUs = now;
    lastGoodPeriodUs = period;

    periodSumUs += period;
    periodCount++;
}

// ============================================================================
// Begin
// ============================================================================

void RPMInput::begin()
{
    pinMode(
        pin,
        INPUT_PULLUP);

    noInterrupts();

    lastPulseUs = 0;
    lastGoodPeriodUs = 0;

    periodSumUs = 0;
    periodCount = 0;

    interrupts();

    currentRPM = 0;

    attachInterrupt(
        digitalPinToInterrupt(pin),
        isr,
        FALLING);
}

// ============================================================================
// Update
// ============================================================================

void RPMInput::update()
{
    uint32_t sum;
    uint16_t count;
    uint32_t lastPulse;

    // ------------------------------------------------------------------------
    // Snapshot ISR data
    // ------------------------------------------------------------------------

    noInterrupts();

    sum = periodSumUs;
    count = periodCount;
    lastPulse = lastPulseUs;

    periodSumUs = 0;
    periodCount = 0;

    interrupts();

    const uint32_t now =
        micros();

    // ------------------------------------------------------------------------
    // Engine stopped / signal lost
    // ------------------------------------------------------------------------

    if (
        lastPulse == 0 ||
        (now - lastPulse) > TIMEOUT_US)
    {
        currentRPM = 0;
        return;
    }

    // No new complete period yet.
    if (count == 0)
    {
        return;
    }

    // ------------------------------------------------------------------------
    // Average accepted ignition periods
    // ------------------------------------------------------------------------

    const float averagePeriodUs =
        static_cast<float>(sum) /
        static_cast<float>(count);

    if (averagePeriodUs <= 0.0f)
    {
        return;
    }

    // ------------------------------------------------------------------------
    // Calculate RPM
    // ------------------------------------------------------------------------

    const float calculatedRPM =
        60000000.0f /
        (averagePeriodUs *
         static_cast<float>(pulsesPerRev));

    const uint16_t newRPM =
        static_cast<uint16_t>(
            calculatedRPM + 0.5f);

    // First valid measurement:
    // don't slowly ramp from zero.
    if (currentRPM == 0)
    {
        currentRPM = newRPM;
        return;
    }

    // ------------------------------------------------------------------------
    // Light adaptive smoothing
    // ------------------------------------------------------------------------
    //
    // Importantly, we DO NOT reject large RPM changes here anymore.
    //
    // The ISR deals with false ignition edges.
    // This part is only for visual smoothness.
    //

    const int32_t delta =
        static_cast<int32_t>(newRPM) -
        static_cast<int32_t>(currentRPM);

    const uint32_t rpmDifference =
        static_cast<uint32_t>(
            abs(delta));

    const float alpha =
        (rpmDifference > MOVING_THRESHOLD_RPM)
            ? MOVING_ALPHA
            : STEADY_ALPHA;

    currentRPM =
        static_cast<uint16_t>(
            alpha *
                static_cast<float>(newRPM) +
            (1.0f - alpha) *
                static_cast<float>(currentRPM) +
            0.5f);
}

// ============================================================================
// Get RPM
// ============================================================================

uint16_t RPMInput::rpm() const
{
    return currentRPM;
}