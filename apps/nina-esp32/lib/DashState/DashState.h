#pragma once
#include <Arduino.h>

struct DashState {
    float fuelPct  = 0.0f;
    float tempPct  = 0.0f;
    float odoKm    = 0.0f;
    float tripKm   = 0.0f;
    float speedKph = 0.0f;
};
