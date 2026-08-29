#ifndef NINA_ANALOGSENSORS_H
#define NINA_ANALOGSENSORS_H

#pragma once
#include <Arduino.h>

struct TempRPoint
{
    float resistance;
    int tempC;
};

inline constexpr TempRPoint TEMP_R_TABLE[] =
    {
        {2000.0f, 20},
        {300.0f, 80},
        {220.0f, 90},
        {125.0f, 100},
};

inline constexpr size_t TEMP_R_TABLE_SIZE =
    sizeof(TEMP_R_TABLE) /
    sizeof(TEMP_R_TABLE[0]);

struct FuelAdcPoint
{
    uint16_t adc;
    uint8_t percent;
};

inline constexpr FuelAdcPoint FUEL_TABLE[] =
    {
        {1724, 0},
        {1484, 25},
        {1188, 50},
        {819, 75},
        {341, 100},
};

inline constexpr size_t FUEL_TABLE_SIZE =
    sizeof(FUEL_TABLE) /
    sizeof(FUEL_TABLE[0]);

struct AnalogSensorsConfig
{
    uint8_t adcBits;
    uint16_t adcMax;

    float senderSupplyV;

    float tempPullupOhms;
    float fuelPullupOhms;

    const TempRPoint *tempRTable;
    size_t tempRTableSize;

    const FuelAdcPoint *fuelTable;
    size_t fuelTableSize;

    int tempMinC;
    int tempMaxC;
};

class AnalogSensors
{
public:
    struct Pins
    {
        uint8_t temp;
        uint8_t fuel;
    };

    AnalogSensors(
        const Pins &pins,
        const AnalogSensorsConfig &config);

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

    float adcToResistance(
        float adc,
        float pullupOhms) const;

    int interpolateTempFromResistance(
        float resistance) const;

    float interpolateFuelFromAdc(
        float adc) const;
};

#endif