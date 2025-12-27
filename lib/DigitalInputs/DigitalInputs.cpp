//
// Created by Miftari Simel on 27. 12. 2025..
//

#include "DigitalInputs.h"

DigitalInputs::DigitalInputs(const Pins& pins)
  : pins(pins) {}

void DigitalInputs::begin() {
    pinMode(pins.brake,      INPUT_PULLUP);
    pinMode(pins.oil,        INPUT_PULLUP);
    pinMode(pins.indicators, INPUT_PULLUP);
    pinMode(pins.highBeam,   INPUT_PULLUP);
    pinMode(pins.lights,     INPUT_PULLUP);
    pinMode(pins.fog,        INPUT_PULLUP);
    pinMode(pins.battery,    INPUT_PULLUP);
}

void DigitalInputs::update() {
    brakeState      = !digitalRead(pins.brake);
    oilState        = !digitalRead(pins.oil);
    indicatorsState = !digitalRead(pins.indicators);
    highBeamState   = !digitalRead(pins.highBeam);
    lightsState     = !digitalRead(pins.lights);
    fogState        = !digitalRead(pins.fog);
    batteryState    = !digitalRead(pins.battery);
}

bool DigitalInputs::brake() const      { return brakeState; }
bool DigitalInputs::oil() const        { return oilState; }
bool DigitalInputs::indicators() const { return indicatorsState; }
bool DigitalInputs::highBeam() const   { return highBeamState; }
bool DigitalInputs::lights() const     { return lightsState; }
bool DigitalInputs::fog() const        { return fogState; }
bool DigitalInputs::battery() const    { return batteryState; }