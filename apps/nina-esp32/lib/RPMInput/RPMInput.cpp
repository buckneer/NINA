#include "RPMInput.h"

volatile uint32_t RPMInput::lastPulseUs = 0;
volatile uint32_t RPMInput::periodSumUs = 0;
volatile uint16_t RPMInput::periodCount = 0;

RPMInput::RPMInput(uint8_t p, uint8_t ppr)
    : pin(p),
      pulsesPerRev(ppr)
{
}

void IRAM_ATTR RPMInput::isr()
{
    const uint32_t now = micros();

    // First pulse: just establish a reference.
    if (lastPulseUs == 0)
    {
        lastPulseUs = now;
        return;
    }

    const uint32_t period = now - lastPulseUs;

    // Reject ignition ringing/noise.
    if (period < MIN_PULSE_US)
    {
        return;
    }

    // Engine was stopped / signal disappeared.
    // Treat this as the first pulse of a new run.
    if (period > TIMEOUT_US)
    {
        lastPulseUs = now;
        return;
    }

    lastPulseUs = now;

    periodSumUs += period;
    periodCount++;
}

void RPMInput::begin()
{
    pinMode(pin, INPUT_PULLUP);

    lastPulseUs = 0;
    periodSumUs = 0;
    periodCount = 0;
    currentRPM = 0;

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

    noInterrupts();

    sum = periodSumUs;
    count = periodCount;
    lastPulse = lastPulseUs;

    periodSumUs = 0;
    periodCount = 0;

    interrupts();

    const uint32_t now = micros();

    // Engine stopped / signal disappeared.
    if (lastPulse == 0 || (now - lastPulse) > TIMEOUT_US)
    {
        currentRPM = 0;
        return;
    }

    // Nothing new to calculate yet.
    if (count == 0)
    {
        return;
    }

    const float averagePeriodUs =
        static_cast<float>(sum) /
        static_cast<float>(count);

    const float calculatedRPM =
        60000000.0f /
        (averagePeriodUs * pulsesPerRev);

    currentRPM = static_cast<uint16_t>(calculatedRPM + 0.5f);
}

uint16_t RPMInput::rpm() const
{
    return currentRPM;
}