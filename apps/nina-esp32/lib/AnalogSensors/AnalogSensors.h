#ifndef NINA_ANALOGSENSORS_H
#define NINA_ANALOGSENSORS_H

#pragma once
#include <Arduino.h>

struct TempRPoint
{
    float resistance;
    int tempC;
};

struct AnalogSensorsConfig
{
    uint8_t adcBits;
    uint16_t adcMax;

    float senderSupplyV;

    float tempPullupOhms;
    float fuelPullupOhms;

    const TempRPoint *tempRTable;
    size_t tempRTableSize;

    int tempMinC;
    int tempMaxC;

    float fuelEmptyOhms;
    float fuelFullOhms;
};

class AnalogSensors
{
public:
    struct Pins
    {
        uint8_t temp;
        uint8_t fuel;
    };

    AnalogSensors(const Pins &pins, const AnalogSensorsConfig &config);

    void begin();
    void update();

    int16_t tempC() const;
    uint8_t tempPercent() const;

    uint8_t fuelPercent() const;

    float tempResistanceOhms() const;
    float fuelResistanceOhms() const;

private:
    Pins pins;
    AnalogSensorsConfig config;

    float tempFiltered = 0.0f;
    float fuelFiltered = 0.0f;

    bool filterInitialized = false;

    float adcToResistance(float adc, float pullupOhms) const;
    int interpolateTempFromResistance(float resistance) const;
};

#endif