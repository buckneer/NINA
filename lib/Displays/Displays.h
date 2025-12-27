//
// Created by Miftari Simel on 27. 12. 2025..
//

#ifndef NINA_DISPLAYS_H
#define NINA_DISPLAYS_H


#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

class Displays {
public:
    Displays(
      Adafruit_SSD1306& fuelDisplay,
      Adafruit_SSD1306& tempDisplay,
      hd44780_I2Cexp& lcdDisplay
    );

    void begin();

    void showFuel(uint8_t pct);
    void showTemp(uint8_t pct);

    void showLCD(const char* line1);
    void showLCD(const char* line1, const char* line2);

private:
    void drawBar(Adafruit_SSD1306& disp, uint8_t pct);

    Adafruit_SSD1306& fuel;
    Adafruit_SSD1306& temp;
    hd44780_I2Cexp& lcd;
};

#endif //NINA_DISPLAYS_H