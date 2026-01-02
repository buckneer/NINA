//
// Created by Miftari Simel on 27. 12. 2025..
//

#ifndef NINA_SPEEDO_H
#define NINA_SPEEDO_H

#pragma once
#include <Arduino.h>
#include <Multiplex.h>

class Speedo
{
public:
    Speedo(Multiplex<2> &mux);

    void begin();
    void setSpeed(uint16_t speed);

private:
    static void render(uint8_t channel, void *ctx, uint8_t *regs);

    uint8_t digits[3] = {0, 0, 0};
    Multiplex<2> &multiplex;

    static const uint8_t segmentStates[10];
    static const uint8_t digitStates[3];
};

#endif // NINA_SPEEDO_H