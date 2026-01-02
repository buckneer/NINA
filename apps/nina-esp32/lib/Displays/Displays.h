//
// Created by Miftari Simel on 27. 12. 2025..
//

#ifndef NINA_DISPLAYS_H
#define NINA_DISPLAYS_H


#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

class Displays {
public:
    Displays(
      Adafruit_SSD1306& fuelDisplay,
      Adafruit_SSD1306& tempDisplay,
      Adafruit_SSD1306* mainOledDisplayPtr  // 128x64 SSD1309 OLED (replaces LCD)
    );

    void begin(bool mainOledAvailable = true, bool fuelOledAvailable = true, bool tempOledAvailable = true);

    void showFuel(uint8_t pct);
    void showTemp(uint8_t pct);

    // Text display on main OLED (128x64) - replaces LCD functionality
    void showText(const char* line1);
    void showText(const char* line1, const char* line2);
    void showText(const char* line1, const char* line2, const char* line3);
    void showText(const char* line1, const char* line2, const char* line3, const char* line4);

    // Main display - shows time, date, trip, and odometer
    void showOdometer(uint32_t km, uint32_t tripKm = 0);

    bool isMainOledConnected() const { return mainOledConnected; }
    bool isFuelOledConnected() const { return fuelOledConnected; }
    bool isTempOledConnected() const { return tempOledConnected; }

private:
    void drawBar(Adafruit_SSD1306& disp, uint8_t pct);

    Adafruit_SSD1306& fuel;
    Adafruit_SSD1306& temp;
    Adafruit_SSD1306* mainOled;  // 128x64 main display
    bool mainOledConnected = false;
    bool fuelOledConnected = false;
    bool tempOledConnected = false;
};

#endif //NINA_DISPLAYS_H