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
        OIL         = 0,  // QA → D25
        BATTERY     = 1,  // QB → D26
        BRAKES      = 2,  // QC → D27
        LOW_FUEL    = 3,  // QD → D28
        INDICATORS  = 4,  // QE → D29
        HEADLIGHTS  = 5,  // QF → D30
        FOG_LIGHTS  = 6,  // QG → D31
        HIGH_BEAM   = 7   // QH → D32
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