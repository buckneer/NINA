//
// Created by Miftari Simel on 27. 12. 2025..
//
#include "RPM.h"

RPMMeter::RPMMeter(Multiplex<4>& mux)
  : multiplex(mux) {}

void RPMMeter::begin() {
    multiplex.setRenderer(render, this);
    multiplex.start();
}

void RPMMeter::setRPM(uint16_t rpm) {
    rpm = constrain(rpm, 0, 8000);

    uint8_t ledsToLight = map(rpm, 0, 8000, 0, TOTAL_LEDS);

    for (uint8_t i = 0; i < TOTAL_LEDS; i++) {
        ledStates[i] = (i < ledsToLight);
    }
}

void RPMMeter::render(uint8_t channel, void* ctx) {
    auto* self = static_cast<RPMMeter*>(ctx);
    auto& sr = self->multiplex.shiftRegister();

    if (channel >= CHANNELS) return;

    // Clear previous frame
    sr.setAllLow();

    // Each channel corresponds to one shift register (8 LEDs)
    uint8_t baseIndex = channel * 8;

    for (uint8_t bit = 0; bit < 8; bit++) {
        uint8_t ledIndex = baseIndex + bit;

        if (ledIndex < TOTAL_LEDS) {
            sr.set(channel * 8 + bit, self->ledStates[ledIndex]);
        }
    }
}