//
// Created by Miftari Simel on 27. 12. 2025..
//

#include "Speedo.h"

// Segment states for digits 0–9 (Common Cathode)
const uint8_t Speedo::segmentStates[10] = {
    0b11000000, // 0
    0b11111001, // 1
    0b10100100, // 2
    0b10110000, // 3
    0b10011001, // 4
    0b10010010, // 5
    0b10000010, // 6
    0b11111000, // 7
    0b10000000, // 8
    0b10010000  // 9
  };

const uint8_t Speedo::digitStates[3] = {
    0b00000001, // Hundreds
    0b00000010, // Tens
    0b00000100  // Units
  };

Speedo::Speedo(Multiplex<2>& mux)
  : multiplex(mux) {}

void Speedo::begin() {
    multiplex.setRenderer(render, this);
    multiplex.start();
}

void Speedo::setSpeed(uint16_t speed) {
    speed = constrain(speed, 0, 999);
    digits[0] = speed / 100;
    digits[1] = (speed / 10) % 10;
    digits[2] = speed % 10;
}

void Speedo::render(uint8_t channel, void* ctx) {
    auto* self = static_cast<Speedo*>(ctx);

    if (channel >= 3) return;

    auto& sr = self->multiplex.shiftRegister();

    sr.setAllLow();
    sr.set(0, segmentStates[self->digits[channel]]);
    sr.set(1, digitStates[channel]);
}