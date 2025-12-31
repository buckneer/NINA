//
// Created by Miftari Simel on 27. 12. 2025..
//
#include "RPM.h"

RPMMeter::RPMMeter(Multiplex<4> &mux)
    : multiplex(mux) {}

void RPMMeter::begin()
{
    multiplex.setRenderer(render, this);
    multiplex.start();
}

void RPMMeter::setRPM(uint16_t rpm)
{
    rpm = constrain(rpm, 0, 8000);

    uint8_t ledsToLight = map(rpm, 0, 8000, 0, TOTAL_LEDS);

    for (uint8_t i = 0; i < TOTAL_LEDS; i++)
    {
        ledStates[i] = (i < ledsToLight);
    }
}

void RPMMeter::render(uint8_t /*channel*/, void *ctx, uint8_t *regs)
{
    auto *self = static_cast<RPMMeter *>(ctx);

    uint8_t ledIndex = 0;

    // Rebuild the ENTIRE shift register chain every tick
    for (uint8_t reg = 0; reg < CHANNELS; reg++)
    {
        uint8_t value = 0;
        uint8_t bits = (reg == CHANNELS - 1) ? 5 : 8; // last register partial

        for (uint8_t bit = 0; bit < bits; bit++)
        {
            if (ledIndex < TOTAL_LEDS && self->ledStates[ledIndex])
            {
                value |= (1 << bit); // LSB-first, matches old sr.set()
            }
            ledIndex++;
        }

        regs[reg] = value;
    }
}
