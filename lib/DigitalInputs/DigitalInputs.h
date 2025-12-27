//
// Created by Miftari Simel on 27. 12. 2025..
//

#ifndef NINA_DIGITALINPUTS_H
#define NINA_DIGITALINPUTS_H


#pragma once
#include <Arduino.h>

class DigitalInputs {
public:
    struct Pins {
        uint8_t brake;
        uint8_t oil;
        uint8_t indicators;
        uint8_t highBeam;
        uint8_t lights;
        uint8_t fog;
        uint8_t battery;
    };

    explicit DigitalInputs(const Pins& pins);

    void begin();
    void update();

    bool brake() const;
    bool oil() const;
    bool indicators() const;
    bool highBeam() const;
    bool lights() const;
    bool fog() const;
    bool battery() const;

private:
    Pins pins;

    bool brakeState = false;
    bool oilState = false;
    bool indicatorsState = false;
    bool highBeamState = false;
    bool lightsState = false;
    bool fogState = false;
    bool batteryState = false;
};

#endif //NINA_DIGITALINPUTS_H