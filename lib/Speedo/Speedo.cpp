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
    
    // Blank leading zeros: only show digit if it's non-zero or if it's the units digit
    // For speed 0, only show units digit (0), blank hundreds and tens
    bool shouldDisplay = false;
    if (channel == 2) {
        // Always show units digit (even if 0)
        shouldDisplay = true;
    } else if (channel == 1) {
        // Show tens digit if it's non-zero OR if hundreds is non-zero
        shouldDisplay = (self->digits[1] != 0) || (self->digits[0] != 0);
    } else if (channel == 0) {
        // Show hundreds digit only if it's non-zero
        shouldDisplay = (self->digits[0] != 0);
    }
    
    // Only enable and set segments if we should display this digit
    if (shouldDisplay) {
        // Set segment pattern (table is correct per user)
        sr.set(0, segmentStates[self->digits[channel]]);
        // Enable this digit
        sr.set(1, digitStates[channel]);
    } else {
        // Don't enable the digit at all (leave digit enable at 0)
        // Don't set segments either (leave at 0 from setAllLow)
        // This should blank the digit completely
    }
}