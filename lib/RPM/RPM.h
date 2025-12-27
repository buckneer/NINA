//
// Created by Miftari Simel on 27. 12. 2025..
//

#ifndef NINA_RPM_H
#define NINA_RPM_H
#pragma once
#include <Arduino.h>
#include <Multiplex.h>

class RPMMeter {
public:
    static constexpr uint8_t TOTAL_LEDS = 29;
    static constexpr uint8_t CHANNELS   = 4;

    RPMMeter(Multiplex<4>& mux);

    void begin();
    void setRPM(uint16_t rpm);

private:
    static void render(uint8_t channel, void* ctx);

    bool ledStates[TOTAL_LEDS] = {false};
    Multiplex<4>& multiplex;
};
#endif //NINA_RPM_H