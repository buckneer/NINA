#include "DashLights.h"

DashLights::DashLights(Multiplex<1> &mux)
    : multiplex(mux)
{
}

void DashLights::begin()
{
    pendingState = 0x00;
    activeState = 0x00;

    multiplex.setRenderer(render, this);

    // Establish known physical state immediately.
    multiplex.registers()[0] = 0x00;
    multiplex.flush();

    // Continuous refresh, like the original working implementation.
    multiplex.start();
}

void DashLights::setLight(Light light, bool on)
{
    // Original polarity — confirmed correct.
    bitWrite(pendingState, light, on);
}

void DashLights::update()
{
    // Commit the complete state once per loop.
    activeState = pendingState;
}

void DashLights::render(uint8_t channel, void *ctx, uint8_t *regs)
{
    if (channel != 0)
        return;

    auto *self = static_cast<DashLights *>(ctx);
    regs[0] = self->activeState;
}

void DashLights::allOff()
{
    pendingState = 0x00;
    activeState = 0x00;
}

void DashLights::allOn()
{
    pendingState = 0xFF;
    activeState = 0xFF;
}

void DashLights::setOil(bool on) { setLight(OIL, on); }
void DashLights::setBrakes(bool on) { setLight(BRAKES, on); }
void DashLights::setLowFuel(bool on) { setLight(LOW_FUEL, on); }
void DashLights::setBattery(bool on) { setLight(BATTERY, on); }
void DashLights::setIndicators(bool on) { setLight(INDICATORS, on); }
void DashLights::setHeadlights(bool on) { setLight(HEADLIGHTS, on); }
void DashLights::setFogLights(bool on) { setLight(FOG_LIGHTS, on); }
void DashLights::setHighBeam(bool on) { setLight(HIGH_BEAM, on); }