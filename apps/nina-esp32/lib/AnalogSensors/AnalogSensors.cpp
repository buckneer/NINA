#include "AnalogSensors.h"
#include <math.h>

AnalogSensors::AnalogSensors(
    const Pins &p,
    const AnalogSensorsConfig &c)
    : pins(p),
      config(c)
{
}

void AnalogSensors::begin()
{
  analogReadResolution(config.adcBits);

  analogSetPinAttenuation(
      pins.temp,
      ADC_11db);

  analogSetPinAttenuation(
      pins.fuel,
      ADC_11db);
}

void AnalogSensors::update()
{
  uint16_t rawTemp =
      analogRead(pins.temp);

  uint16_t rawFuel =
      analogRead(pins.fuel);

  rawTemp =
      min(rawTemp, config.adcMax);

  rawFuel =
      min(rawFuel, config.adcMax);

  if (!filterInitialized)
  {
    tempFiltered = rawTemp;
    fuelFiltered = rawFuel;

    filterInitialized = true;
    return;
  }

  tempFiltered =
      tempFiltered * 0.9f +
      rawTemp * 0.1f;

  fuelFiltered =
      fuelFiltered * 0.9f +
      rawFuel * 0.1f;
}

float AnalogSensors::adcToResistance(
    float adc,
    float pullupOhms) const
{
  // Sender is a resistor to ground with a 3.3 V pull-up.
  if (adc >= config.adcMax - 1)
    return INFINITY;

  if (adc <= 0.0f)
    return 0.0f;

  const float ratio =
      adc /
      static_cast<float>(config.adcMax);

  return pullupOhms *
         ratio /
         (1.0f - ratio);
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

// ============================================================================
// Temperature
// ============================================================================

int AnalogSensors::interpolateTempFromResistance(
    float r) const
{
  if (
      config.tempRTable == nullptr ||
      config.tempRTableSize < 2 ||
      !isfinite(r) ||
      r <= 0.0f)
  {
    return config.tempMinC;
  }

  for (
      size_t i = 0;
      i < config.tempRTableSize - 1;
      i++)
  {
    const auto &p1 =
        config.tempRTable[i];

    const auto &p2 =
        config.tempRTable[i + 1];

    const bool between =
        (r >= p1.resistance &&
         r <= p2.resistance) ||
        (r <= p1.resistance &&
         r >= p2.resistance);

    if (!between)
      continue;

    // NTC resistance is nonlinear, so interpolate
    // using logarithmic resistance.
    const float logR =
        logf(r);

    const float logR1 =
        logf(p1.resistance);

    const float logR2 =
        logf(p2.resistance);

    const float ratio =
        (logR - logR1) /
        (logR2 - logR1);

    const float temp =
        p1.tempC +
        ratio *
            (p2.tempC - p1.tempC);

    return static_cast<int>(
        roundf(temp));
  }

  // Outside calibrated range:
  // clamp to nearest table endpoint.

  const float firstDistance =
      fabsf(
          r -
          config.tempRTable[0].resistance);

  const float lastDistance =
      fabsf(
          r -
          config.tempRTable[config.tempRTableSize - 1]
              .resistance);

  if (firstDistance < lastDistance)
  {
    return config.tempRTable[0].tempC;
  }

  return config.tempRTable[config.tempRTableSize - 1]
      .tempC;
}

int16_t AnalogSensors::tempC() const
{
  if (
      config.tempRTable != nullptr &&
      config.tempRTableSize >= 2)
  {
    const int t =
        interpolateTempFromResistance(
            tempResistanceOhms());

    return constrain(
        t,
        config.tempMinC,
        config.tempMaxC);
  }

  // Fallback if no table exists.
  const float ratio =
      tempFiltered /
      static_cast<float>(
          config.adcMax);

  const int t =
      config.tempMaxC -
      static_cast<int>(
          ratio *
          (config.tempMaxC -
           config.tempMinC));

  return constrain(
      t,
      config.tempMinC,
      config.tempMaxC);
}

uint8_t AnalogSensors::tempPercent() const
{
  const int t =
      tempC();

  const float pct =
      static_cast<float>(
          t - config.tempMinC) /
      static_cast<float>(
          config.tempMaxC -
          config.tempMinC) *
      100.0f;

  return constrain(
      static_cast<int>(
          roundf(pct)),
      0,
      100);
}

// ============================================================================
// Fuel
// ============================================================================

float AnalogSensors::interpolateFuelFromAdc(
    float adc) const
{
  if (
      config.fuelTable == nullptr ||
      config.fuelTableSize < 2)
  {
    return 0.0f;
  }

  for (
      size_t i = 0;
      i < config.fuelTableSize - 1;
      i++)
  {
    const auto &p1 =
        config.fuelTable[i];

    const auto &p2 =
        config.fuelTable[i + 1];

    const bool between =
        (adc >= p1.adc &&
         adc <= p2.adc) ||
        (adc <= p1.adc &&
         adc >= p2.adc);

    if (!between)
      continue;

    const float adcSpan =
        static_cast<float>(p2.adc) -
        static_cast<float>(p1.adc);

    if (fabsf(adcSpan) < 0.001f)
      return p1.percent;

    const float ratio =
        (adc -
         static_cast<float>(p1.adc)) /
        adcSpan;

    return static_cast<float>(p1.percent) +
           ratio *
               (static_cast<float>(
                    p2.percent) -
                static_cast<float>(
                    p1.percent));
  }

  // Outside table range:
  // clamp to whichever endpoint is closest.

  const auto &first =
      config.fuelTable[0];

  const auto &last =
      config.fuelTable[config.fuelTableSize - 1];

  if (
      fabsf(adc - first.adc) <
      fabsf(adc - last.adc))
  {
    return first.percent;
  }

  return last.percent;
}

uint8_t AnalogSensors::fuelPercent() const
{
  if (
      config.fuelTable == nullptr ||
      config.fuelTableSize < 2)
  {
    return 0;
  }

  const float pct =
      interpolateFuelFromAdc(
          fuelFiltered);

  return constrain(
      static_cast<int>(
          roundf(pct)),
      0,
      100);
}