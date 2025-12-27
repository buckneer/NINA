#include "AnalogSensors.h"
#include <math.h>

// -------------------------------------------------
// Constructor
// -------------------------------------------------

AnalogSensors::AnalogSensors(const Pins &p, const AnalogSensorsConfig &c)
    : pins(p), config(c) {}

// -------------------------------------------------
// Init
// -------------------------------------------------

void AnalogSensors::begin()
{
  analogReadResolution(config.adcBits);

  // Make ADC range truthful (0–3.3V)
  analogSetPinAttenuation(pins.temp, ADC_11db);
  analogSetPinAttenuation(pins.fuel, ADC_11db);
}

// -------------------------------------------------
// Update (call every loop)
// -------------------------------------------------

void AnalogSensors::update()
{
  uint16_t rawTemp = analogRead(pins.temp);
  uint16_t rawFuel = analogRead(pins.fuel);

  // Simple low-pass filter
  tempFiltered = tempFiltered * 0.9f + rawTemp * 0.1f;
  fuelFiltered = fuelFiltered * 0.9f + rawFuel * 0.1f;
}

// =================================================
// ============ TEMPERATURE (OEM YUGO, ADC LUT)
// =================================================

// Linear interpolation in ADC-voltage domain
int AnalogSensors::interpolateTempFromVoltage(float v) const
{
  for (size_t i = 0; i < config.tempVTableSize - 1; i++)
  {
    const auto &p1 = config.tempVTable[i];
    const auto &p2 = config.tempVTable[i + 1];

    if (v >= p1.vadc && v <= p2.vadc)
    {
      float t =
          p1.tempC +
          (p2.tempC - p1.tempC) *
              (v - p1.vadc) /
              (p2.vadc - p1.vadc);

      return static_cast<int>(t);
    }
  }

  // Out-of-range safety
  if (v < config.tempVTable[0].vadc)
    return config.tempVTable[0].tempC;

  return config.tempVTable[config.tempVTableSize - 1].tempC;
}

int16_t AnalogSensors::tempC() const
{
  float vadc = (tempFiltered / config.adcMax) * config.adcRefV;

  int t = interpolateTempFromVoltage(vadc);
  return constrain(t, config.tempMinC, config.tempMaxC);
}

// Map temperature to 0–100% (for bar display)
uint8_t AnalogSensors::tempPercent() const
{
  int t = tempC();

  float pct =
      (float)(t - config.tempMinC) /
      (config.tempMaxC - config.tempMinC) * 100.0f;

  return constrain(static_cast<int>(pct), 0, 100);
}

// =================================================
// ================= FUEL SENDER
// =================================================

uint8_t AnalogSensors::fuelPercent() const
{
  float vadc = (fuelFiltered / config.adcMax) * config.adcRefV;

  float pct =
      (vadc - config.fuelAdcVMin) /
      (config.fuelAdcVMax - config.fuelAdcVMin) * 100.0f;

  return constrain(static_cast<int>(pct), 0, 100);
}