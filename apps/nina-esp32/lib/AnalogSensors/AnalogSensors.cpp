#include "AnalogSensors.h"
#include <math.h>

AnalogSensors::AnalogSensors(const Pins &p, const AnalogSensorsConfig &c)
    : pins(p), config(c) {}

void AnalogSensors::begin()
{
  analogReadResolution(config.adcBits);

  analogSetPinAttenuation(pins.temp, ADC_11db);
  analogSetPinAttenuation(pins.fuel, ADC_11db);
}

void AnalogSensors::update()
{
  uint16_t rawTemp = analogRead(pins.temp);
  uint16_t rawFuel = analogRead(pins.fuel);

  rawTemp = min(rawTemp, config.adcMax);
  rawFuel = min(rawFuel, config.adcMax);

  if (!filterInitialized)
  {
    tempFiltered = rawTemp;
    fuelFiltered = rawFuel;
    filterInitialized = true;
    return;
  }

  tempFiltered = tempFiltered * 0.9f + rawTemp * 0.1f;
  fuelFiltered = fuelFiltered * 0.9f + rawFuel * 0.1f;
}

float AnalogSensors::adcToResistance(float adc, float pullupOhms) const
{
  // Sender is a resistor to ground with a 3.3 V pull-up.
  if (adc >= config.adcMax - 1)
    return INFINITY;

  if (adc <= 0.0f)
    return 0.0f;

  float ratio = adc / static_cast<float>(config.adcMax);

  return pullupOhms * ratio / (1.0f - ratio);
}

float AnalogSensors::tempResistanceOhms() const
{
  return adcToResistance(
      tempFiltered,
      config.tempPullupOhms);
}

float AnalogSensors::fuelResistanceOhms() const
{
  return adcToResistance(
      fuelFiltered,
      config.fuelPullupOhms);
}

int AnalogSensors::interpolateTempFromResistance(float r) const
{
  if (!isfinite(r))
    return config.tempMinC;

  for (size_t i = 0; i < config.tempRTableSize - 1; i++)
  {
    const auto &p1 = config.tempRTable[i];
    const auto &p2 = config.tempRTable[i + 1];

    bool between =
        (r >= p1.resistance && r <= p2.resistance) ||
        (r <= p1.resistance && r >= p2.resistance);

    if (!between)
      continue;

    float t =
        p1.tempC +
        (p2.tempC - p1.tempC) *
            (r - p1.resistance) /
            (p2.resistance - p1.resistance);

    return static_cast<int>(t);
  }

  float firstDistance =
      fabsf(r - config.tempRTable[0].resistance);

  float lastDistance =
      fabsf(r - config.tempRTable[config.tempRTableSize - 1].resistance);

  if (firstDistance < lastDistance)
    return config.tempRTable[0].tempC;

  return config.tempRTable[config.tempRTableSize - 1].tempC;
}

int16_t AnalogSensors::tempC() const
{
  // Use real resistance table once calibrated
  if (config.tempRTable != nullptr && config.tempRTableSize >= 2)
  {
    int t = interpolateTempFromResistance(tempResistanceOhms());
    return constrain(t, config.tempMinC, config.tempMaxC);
  }

  // Temporary fallback: low ADC = hotter sender
  float ratio = tempFiltered / static_cast<float>(config.adcMax);

  int t =
      config.tempMaxC -
      static_cast<int>(
          ratio * (config.tempMaxC - config.tempMinC));

  return constrain(t, config.tempMinC, config.tempMaxC);
}

uint8_t AnalogSensors::tempPercent() const
{
  int t = tempC();

  float pct =
      static_cast<float>(t - config.tempMinC) /
      static_cast<float>(config.tempMaxC - config.tempMinC) *
      100.0f;

  return constrain(
      static_cast<int>(pct),
      0,
      100);
}

uint8_t AnalogSensors::fuelPercent() const
{
  float r = fuelResistanceOhms();

  // Use real calibration once available
  if (config.fuelFullOhms != config.fuelEmptyOhms)
  {
    if (!isfinite(r))
      return 0;

    float pct =
        (r - config.fuelEmptyOhms) /
        (config.fuelFullOhms - config.fuelEmptyOhms) *
        100.0f;

    return constrain(
        static_cast<int>(pct),
        0,
        100);
  }

  // Temporary fallback: low ADC = fuller tank
  float ratio =
      fuelFiltered / static_cast<float>(config.adcMax);

  float pct = (1.0f - ratio) * 100.0f;

  return constrain(
      static_cast<int>(pct),
      0,
      100);
}