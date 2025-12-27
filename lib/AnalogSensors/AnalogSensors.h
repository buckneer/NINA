//
// Created by Miftari Simel on 27. 12. 2025..
//

#ifndef NINA_ANALOGSENSORS_H
#define NINA_ANALOGSENSORS_H

#pragma once
#include <Arduino.h>

struct TempVPoint
{
    float vadc; // volts at ESP32 ADC pin (AFTER divider)
    int tempC;  // °C
};

struct AnalogSensorsConfig
{
    // ADC settings
    uint8_t adcBits;
    float adcRefV;
    uint16_t adcMax;

    // Temperature calibration
    const TempVPoint* tempVTable;
    size_t tempVTableSize;
    int tempMinC;
    int tempMaxC;

    // Fuel calibration
    float fuelAdcVMin; // empty
    float fuelAdcVMax; // full
};

class AnalogSensors
{
public:
    struct Pins
    {
        uint8_t temp; // ADC pin for engine temp
        uint8_t fuel; // ADC pin for fuel sender
    };

    AnalogSensors(const Pins &pins, const AnalogSensorsConfig &config);

    void begin();
    void update();

    // Engine temperature
    int16_t tempC() const;       // real temperature
    uint8_t tempPercent() const; // mapped for display

    // Fuel
    uint8_t fuelPercent() const;

private:
    Pins pins;
    AnalogSensorsConfig config;

    float tempFiltered = 0.0f;
    float fuelFiltered = 0.0f;

    int interpolateTempFromVoltage(float voltage) const;
};
#endif // NINA_ANALOGSENSORS_H