#ifndef NINA_MULTIPLEX_H
#define NINA_MULTIPLEX_H

#pragma once

#include <Arduino.h>
#include <Ticker.h>

// Shared by ALL Multiplex<> instances.
// Important because all shift-register banks share SRCLK.
inline portMUX_TYPE gShiftRegisterMux = portMUX_INITIALIZER_UNLOCKED;

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

        digitalWrite(clockPin, LOW);
        digitalWrite(latchPin, LOW);

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

    // Backward-compatible renderer
    void setRenderer(void (*fn)(uint8_t, void *), void *userCtx)
    {
        legacyRenderer = fn;
        renderer = nullptr;
        ctx = userCtx;
    }

    // Preferred renderer
    void setRenderer(RenderFn fn, void *userCtx)
    {
        renderer = fn;
        legacyRenderer = nullptr;
        ctx = userCtx;
    }

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
        portENTER_CRITICAL(&gShiftRegisterMux);

        digitalWrite(latchPin, LOW);

        for (int i = NUM_REGS - 1; i >= 0; --i)
        {
            shiftOut(
                dataPin,
                clockPin,
                MSBFIRST,
                regs[i]);
        }

        // Latch the completed data.
        digitalWrite(latchPin, HIGH);

        // Safe idle states.
        digitalWrite(latchPin, LOW);
        digitalWrite(dataPin, LOW);

        portEXIT_CRITICAL(&gShiftRegisterMux);
    }

private:
    void onTick()
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

        ++currentChannel;

        if (currentChannel >= channels)
        {
            currentChannel = 0;
        }
    }

    uint8_t dataPin;
    uint8_t clockPin;
    uint8_t latchPin;

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