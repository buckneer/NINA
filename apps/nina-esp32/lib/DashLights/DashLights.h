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
        BATTERY = 0,
        BRAKES = 1,
        OIL = 2,
        LOW_FUEL = 3,
        INDICATORS = 4,
        HEADLIGHTS = 5,
        FOG_LIGHTS = 6,
        HIGH_BEAM = 7
    };

    DashLights(Multiplex<1> &mux);

    void begin();
    void update();

    void setLight(Light light, bool on);

    void setOil(bool on);
    void setBrakes(bool on);
    void setLowFuel(bool on);
    void setBattery(bool on);
    void setIndicators(bool on);
    void setHeadlights(bool on);
    void setFogLights(bool on);
    void setHighBeam(bool on);

    void allOff();
    void allOn();

private:
    static void render(uint8_t channel, void *ctx, uint8_t *regs);

    uint8_t pendingState = 0x00;
    volatile uint8_t activeState = 0x00;

    Multiplex<1> &multiplex;
};

#endif