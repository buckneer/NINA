//
// Created by Miftari Simel on 27. 12. 2025..
//

#ifndef NINA_MULTIPLEX_H
#define NINA_MULTIPLEX_H


#pragma once

#include <Arduino.h>
#include <Ticker.h>
#include <ShiftRegister74HC595.h>

template<uint8_t NUM_REGS>
class Multiplex {
public:
    using RenderFn = void (*)(uint8_t channel, void *ctx);

    Multiplex(
        uint8_t dataPin,
        uint8_t clockPin,
        uint8_t latchPin,
        uint8_t channels,
        uint32_t refreshMs
    )
        : sr(dataPin, clockPin, latchPin),
          channels(channels),
          refreshMs(refreshMs) {
    }

    void begin() {
        sr.setAllLow();
    }

    void start() {
        ticker.attach_ms(
            refreshMs,
            +[](Multiplex *self) {
                self->onTick();
            },
            this
        );
    }

    void stop() {
        ticker.detach();
    }

    void setRenderer(RenderFn fn, void *userCtx) {
        renderer = fn;
        ctx = userCtx;
    }

    ShiftRegister74HC595<NUM_REGS> &shiftRegister() {
        return sr;
    }

private:
    void IRAM_ATTR onTick() {
        if (!renderer) return;

        renderer(currentChannel, ctx);

        currentChannel++;
        if (currentChannel >= channels)
            currentChannel = 0;
    }

    ShiftRegister74HC595<NUM_REGS> sr;
    Ticker ticker;

    RenderFn renderer = nullptr;
    void *ctx = nullptr;

    uint8_t currentChannel = 0;
    uint8_t channels;
    uint32_t refreshMs;
};

#endif //NINA_MULTIPLEX_H
