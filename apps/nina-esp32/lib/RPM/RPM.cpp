//
// Created by Miftari Simel on 27. 12. 2025..
//

#include "RPM.h"

RPMMeter::RPMMeter(
    Multiplex<4> &mux,
    uint16_t maxRPM)
    : multiplex(mux),
      maxRPM(maxRPM)
{
}

void RPMMeter::begin()
{
    multiplex.setRenderer(render, this);
    multiplex.start();

    requestedRPM = 0;
    sweeping = false;

    applyRPM(0);
}

void RPMMeter::setRPM(uint16_t rpm)
{
    // Always remember the newest real RPM.
    requestedRPM = constrain(
        rpm,
        0,
        maxRPM);

    // During the startup sweep, the animation owns the LEDs.
    if (sweeping)
    {
        return;
    }

    applyRPM(requestedRPM);
}

void RPMMeter::startSweep()
{
    sweepStartMs = millis();
    sweeping = true;

    applyRPM(0);
}

void RPMMeter::update()
{
    if (!sweeping)
    {
        return;
    }

    const uint32_t elapsed =
        millis() - sweepStartMs;

    // =========================================================
    // Sweep UP
    // =========================================================

    if (elapsed < SWEEP_UP_MS)
    {
        const uint32_t sweepRPM =
            static_cast<uint32_t>(maxRPM) *
            elapsed /
            SWEEP_UP_MS;

        applyRPM(
            static_cast<uint16_t>(sweepRPM));

        return;
    }

    // =========================================================
    // Hold at maximum
    // =========================================================

    const uint32_t holdEnd =
        SWEEP_UP_MS +
        SWEEP_HOLD_MS;

    if (elapsed < holdEnd)
    {
        applyRPM(maxRPM);
        return;
    }

    // =========================================================
    // Sweep DOWN
    // =========================================================

    const uint32_t sweepEnd =
        holdEnd +
        SWEEP_DOWN_MS;

    if (elapsed < sweepEnd)
    {
        const uint32_t downElapsed =
            elapsed - holdEnd;

        const uint32_t remaining =
            SWEEP_DOWN_MS - downElapsed;

        const uint32_t sweepRPM =
            static_cast<uint32_t>(maxRPM) *
            remaining /
            SWEEP_DOWN_MS;

        applyRPM(
            static_cast<uint16_t>(sweepRPM));

        return;
    }

    // =========================================================
    // Finished
    // =========================================================

    sweeping = false;

    // Immediately hand control back to the latest real RPM.
    applyRPM(requestedRPM);
}

bool RPMMeter::isSweeping() const
{
    return sweeping;
}

void RPMMeter::applyRPM(uint16_t rpm)
{
    rpm = constrain(
        rpm,
        0,
        maxRPM);

    const uint8_t ledsToLight =
        map(
            rpm,
            0,
            maxRPM,
            0,
            TOTAL_LEDS);

    for (uint8_t i = 0; i < TOTAL_LEDS; i++)
    {
        ledStates[i] =
            (i < ledsToLight);
    }
}

void RPMMeter::render(
    uint8_t /*channel*/,
    void *ctx,
    uint8_t *regs)
{
    auto *self =
        static_cast<RPMMeter *>(ctx);

    uint8_t ledIndex = 0;

    // Rebuild the entire shift-register chain every tick.
    for (uint8_t reg = 0; reg < CHANNELS; reg++)
    {
        uint8_t value = 0;

        const uint8_t bits =
            (reg == CHANNELS - 1)
                ? 5
                : 8;

        for (uint8_t bit = 0; bit < bits; bit++)
        {
            if (
                ledIndex < TOTAL_LEDS &&
                self->ledStates[ledIndex])
            {
                value |= (1 << bit);
            }

            ledIndex++;
        }

        regs[reg] = value;
    }
}