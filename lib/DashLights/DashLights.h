//
// Created by Miftari Simel on 27. 12. 2025..
//

#ifndef NINA_DASHLIGHTS_H
#define NINA_DASHLIGHTS_H


#pragma once
#include <Arduino.h>
#include <Multiplex.h>

class DashLights {
public:
    enum Light : uint8_t {
        BRAKES      = 0,
        OIL         = 1,
        LOW_FUEL    = 2,
        BATTERY     = 3,
        INDICATORS  = 4,
        HEADLIGHTS  = 5,
        FOG_LIGHTS  = 6,
        HIGH_BEAM   = 7
      };

    DashLights(Multiplex<1>& mux);

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
    static void render(uint8_t channel, void* ctx);

    uint8_t shiftState = 0;
    Multiplex<1>& multiplex;
};

#endif //NINA_DASHLIGHTS_H