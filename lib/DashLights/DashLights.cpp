//
// Created by Miftari Simel on 27. 12. 2025..
//

#include "DashLights.h"

DashLights::DashLights(Multiplex<1>& mux)
  : multiplex(mux) {}

void DashLights::begin() {
    multiplex.setRenderer(render, this);
    multiplex.start();
}

void DashLights::setLight(Light light, bool on) {
    bitWrite(shiftState, light, on);
}

void DashLights::render(uint8_t channel, void* ctx) {
    auto* self = static_cast<DashLights*>(ctx);
    if (channel != 0) return;

    auto& sr = self->multiplex.shiftRegister();
    sr.setAllLow();

    for (uint8_t bit = 0; bit < 8; bit++) {
        sr.set(bit, bitRead(self->shiftState, bit));
    }
}

// Convenience wrappers
void DashLights::setOil(bool on)        { setLight(OIL, on); }
void DashLights::setBrakes(bool on)     { setLight(BRAKES, on); }
void DashLights::setLowFuel(bool on)    { setLight(LOW_FUEL, on); }
void DashLights::setBattery(bool on)    { setLight(BATTERY, on); }
void DashLights::setIndicators(bool on) { setLight(INDICATORS, on); }
void DashLights::setHeadlights(bool on) { setLight(HEADLIGHTS, on); }
void DashLights::setFogLights(bool on)  { setLight(FOG_LIGHTS, on); }
void DashLights::setHighBeam(bool on)   { setLight(HIGH_BEAM, on); }