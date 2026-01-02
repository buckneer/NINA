#ifndef NINA_MULTIPLEX_H
#define NINA_MULTIPLEX_H

#pragma once

#include <Arduino.h>
#include <Ticker.h>

template <uint8_t NUM_REGS>
class Multiplex
{
public:
    using RenderFn = void (*)(uint8_t channel, void *ctx, uint8_t *regs);

    Multiplex(
        uint8_t dataPin,
        uint8_t clockPin,
        uint8_t latchPin,
        uint8_t channels,
        uint32_t refreshMs)
        : dataPin(dataPin),
          clockPin(clockPin),
          latchPin(latchPin),
          channels(channels),
          refreshMs(refreshMs)
    {
    }

    void begin()
    {
        pinMode(dataPin, OUTPUT);
        pinMode(clockPin, OUTPUT);
        pinMode(latchPin, OUTPUT);
        clear();
        flush();
    }

    void start()
    {
        ticker.attach_ms(
            refreshMs,
            +[](Multiplex *self)
            {
                self->onTick();
            },
            this);
    }

    void stop()
    {
        ticker.detach();
    }

    // 🔥 BACKWARD COMPATIBLE
    void setRenderer(void (*fn)(uint8_t, void *), void *userCtx)
    {
        legacyRenderer = fn;
        ctx = userCtx;
    }

    // ✅ NEW preferred renderer
    void setRenderer(RenderFn fn, void *userCtx)
    {
        renderer = fn;
        ctx = userCtx;
    }

    // --- register helpers
    void clear()
    {
        memset(regs, 0, sizeof(regs));
    }

    uint8_t *registers()
    {
        return regs;
    }

    void flush()
    {
        digitalWrite(latchPin, LOW);
        for (int i = NUM_REGS - 1; i >= 0; i--)
        {
            shiftOut(dataPin, clockPin, MSBFIRST, regs[i]);
        }

        digitalWrite(latchPin, HIGH);
    }

private:
    void IRAM_ATTR onTick()
    {
        clear();

        if (renderer)
        {
            renderer(currentChannel, ctx, regs);
        }
        else if (legacyRenderer)
        {
            legacyRenderer(currentChannel, ctx);
        }

        flush();

        currentChannel++;
        if (currentChannel >= channels)
            currentChannel = 0;
    }

    uint8_t dataPin, clockPin, latchPin;
    uint8_t regs[NUM_REGS]{};

    Ticker ticker;

    RenderFn renderer = nullptr;
    void (*legacyRenderer)(uint8_t, void *) = nullptr;
    void *ctx = nullptr;

    uint8_t currentChannel = 0;
    uint8_t channels;
    uint32_t refreshMs;
};

#endif // NINA_MULTIPLEX_H
