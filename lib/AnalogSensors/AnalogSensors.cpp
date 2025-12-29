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

  // Validate ADC readings (clamp to reasonable range, ignore 0s from connection glitches)
  // For 12-bit ADC, max is 4095 - validate it's not corrupted
  if (rawTemp > config.adcMax) rawTemp = config.adcMax;
  if (rawFuel > config.adcMax) rawFuel = config.adcMax;
  
  // Skip filtering if reading is 0 (connection glitch) - keep last valid value
  // This prevents spikes when wires briefly disconnect
  if (rawTemp > 0) {
    tempFiltered = tempFiltered * 0.9f + rawTemp * 0.1f;
  }
  
  if (rawFuel > 0) {
    fuelFiltered = fuelFiltered * 0.9f + rawFuel * 0.1f;
  }
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
  // Safety check - if filtered value is invalid, return 0
  if (fuelFiltered <= 0 || fuelFiltered > config.adcMax) {
    return 0;
  }
  
  float vadc = (fuelFiltered / (float)config.adcMax) * config.adcRefV;

  // Fuel sensor is INVERTED: full = low voltage, empty = high voltage
  // So we invert the percentage calculation
  float pct =
      (config.fuelAdcVMax - vadc) /
      (config.fuelAdcVMax - config.fuelAdcVMin) * 100.0f;

  return constrain(static_cast<int>(pct), 0, 100);
}