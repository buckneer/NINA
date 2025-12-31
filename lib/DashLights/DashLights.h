#ifndef NINA_DASHLIGHTS_H
#define NINA_DASHLIGHTS_H

#pragma once

#include <Arduino.h>
#include <Multiplex.h>

class DashLights
{
public:
    enum Light : uint8_t
    {
        OIL = 0,        // QA
        BATTERY = 1,    // QB
        BRAKES = 2,     // QC
        LOW_FUEL = 3,   // QD
        INDICATORS = 4, // QE
        HEADLIGHTS = 5, // QF
        FOG_LIGHTS = 6, // QG
        HIGH_BEAM = 7   // QH
    };

    DashLights(Multiplex<1> &mux);

    void begin();

    void setLight(Light light, bool on);

    // Convenience
    void setOil(bool on);
    void setBrakes(bool on);
    void setLowFuel(bool on);
    void setBattery(bool on);
    void setIndicators(bool on);
    void setHeadlights(bool on);
    void setFogLights(bool on);
    void setHighBeam(bool on);

private:
    // 🔥 UPDATED
    static void render(uint8_t channel, void *ctx, uint8_t *regs);

    uint8_t shiftState = 0;
    Multiplex<1> &multiplex;
};

#endif // NINA_DASHLIGHTS_H
