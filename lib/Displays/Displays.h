//
// Created by Miftari Simel on 27. 12. 2025..
//

#ifndef NINA_DISPLAYS_H
#define NINA_DISPLAYS_H


#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <LiquidCrystal_I2C.h>

class Displays {
public:
    Displays(
      Adafruit_SSD1306& fuelDisplay,
      Adafruit_SSD1306& tempDisplay,
      LiquidCrystal_I2C* lcdDisplayPtr
    );

    void begin(bool lcdAvailable = true, bool fuelOledAvailable = true, bool tempOledAvailable = true);

    void showFuel(uint8_t pct);
    void showTemp(uint8_t pct);

    void showLCD(const char* line1);
    void showLCD(const char* line1, const char* line2);

    bool isLCDConnected() const { return lcdConnected; }
    bool isFuelOledConnected() const { return fuelOledConnected; }
    bool isTempOledConnected() const { return tempOledConnected; }

private:
    void drawBar(Adafruit_SSD1306& disp, uint8_t pct);

    Adafruit_SSD1306& fuel;
    Adafruit_SSD1306& temp;
    LiquidCrystal_I2C* lcd;
    bool lcdConnected = false;
    bool fuelOledConnected = false;
    bool tempOledConnected = false;
};

#endif //NINA_DISPLAYS_H