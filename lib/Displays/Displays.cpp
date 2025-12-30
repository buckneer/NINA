//
// Created by Miftari Simel on 27. 12. 2025..
//

#include "Displays.h"

#define W 128
#define H 32

// Bitmaps (unchanged)
static const unsigned char PROGMEM image_Layer_14_bits[] = { 0xf8, 0x20, 0xf8 };
static const unsigned char PROGMEM image_Layer_14_1_bits[] = {
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,
    0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x00,0x00,0x00,0x00,
    0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x00,
    0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x01
  };
static const unsigned char PROGMEM image_Layer_15_bits[] = { 0x70, 0x88, 0x50 };

Displays::Displays(
  Adafruit_SSD1306& fuelDisplay,
  Adafruit_SSD1306& tempDisplay,
  Adafruit_SSD1306* mainOledDisplayPtr
)
  : fuel(fuelDisplay),
    temp(tempDisplay),
    mainOled(mainOledDisplayPtr),
    mainOledConnected(false) {}

void Displays::begin(bool mainOledAvailable, bool fuelOledAvailable, bool tempOledAvailable) {
    fuelOledConnected = fuelOledAvailable;
    tempOledConnected = tempOledAvailable;
    mainOledConnected = mainOledAvailable && (mainOled != nullptr);
    
    // Only initialize displays that are connected
    if (fuelOledConnected) {
        fuel.clearDisplay();
        fuel.display();
    }
    
    if (tempOledConnected) {
        temp.clearDisplay();
        temp.display();
    }
    
    if (mainOledConnected && mainOled != nullptr) {
        mainOled->clearDisplay();
        mainOled->display();
    }
}

void Displays::drawBar(Adafruit_SSD1306& disp, uint8_t pct) {
    disp.clearDisplay();
    disp.drawRoundRect(1, 1, 126, 22, 3, SSD1306_WHITE);

    uint8_t bars = (pct * 10) / 100;
    for (uint8_t i = 0; i < bars; i++) {
        disp.fillRect(5 + 12 * i, 4, 10, 16, SSD1306_WHITE);
    }

    disp.drawBitmap(121, 29, image_Layer_14_bits, 5, 3, SSD1306_WHITE);
    disp.drawBitmap(4, 24, image_Layer_14_1_bits, 120, 4, SSD1306_WHITE);
    disp.drawBitmap(2, 29, image_Layer_15_bits, 5, 3, SSD1306_WHITE);

    disp.display();
}

void Displays::showFuel(uint8_t pct) {
    if (!fuelOledConnected) return; // Skip if Fuel OLED not connected
    drawBar(fuel, pct);
}

void Displays::showTemp(uint8_t pct) {
    if (!tempOledConnected) return; // Skip if Temp OLED not connected
    drawBar(temp, pct);
}

void Displays::showText(const char* line1) {
    if (!mainOledConnected || mainOled == nullptr) return;
    
    mainOled->clearDisplay();
    mainOled->setTextSize(1);
    mainOled->setTextColor(SSD1306_WHITE);
    mainOled->setCursor(0, 0);
    mainOled->println(line1);
    mainOled->display();
}

void Displays::showText(const char* line1, const char* line2) {
    if (!mainOledConnected || mainOled == nullptr) return;
    
    mainOled->clearDisplay();
    mainOled->setTextSize(1);
    mainOled->setTextColor(SSD1306_WHITE);
    mainOled->setCursor(0, 0);
    mainOled->println(line1);
    mainOled->setCursor(0, 16);
    mainOled->println(line2);
    mainOled->display();
}

void Displays::showText(const char* line1, const char* line2, const char* line3) {
    if (!mainOledConnected || mainOled == nullptr) return;
    
    mainOled->clearDisplay();
    mainOled->setTextSize(1);
    mainOled->setTextColor(SSD1306_WHITE);
    mainOled->setCursor(0, 0);
    mainOled->println(line1);
    mainOled->setCursor(0, 16);
    mainOled->println(line2);
    mainOled->setCursor(0, 32);
    mainOled->println(line3);
    mainOled->display();
}

void Displays::showText(const char* line1, const char* line2, const char* line3, const char* line4) {
    if (!mainOledConnected || mainOled == nullptr) return;
    
    mainOled->clearDisplay();
    mainOled->setTextSize(1);
    mainOled->setTextColor(SSD1306_WHITE);
    mainOled->setCursor(0, 0);
    mainOled->println(line1);
    mainOled->setCursor(0, 16);
    mainOled->println(line2);
    mainOled->setCursor(0, 32);
    mainOled->println(line3);
    mainOled->setCursor(0, 48);
    mainOled->println(line4);
    mainOled->display();
}

void Displays::showOdometer(uint32_t km) {
    if (!mainOledConnected || mainOled == nullptr) return;
    
    mainOled->clearDisplay();
    
    // Format the number with thousand separators (e.g., "123,456")
    char kmStr[16];
    if (km >= 1000000) {
        // For 1,000,000+ km, show in millions with one decimal
        snprintf(kmStr, sizeof(kmStr), "%.1f", km / 1000000.0f);
    } else if (km >= 1000) {
        // For 1,000+ km, show with comma separator
        uint32_t thousands = km / 1000;
        uint32_t remainder = km % 1000;
        snprintf(kmStr, sizeof(kmStr), "%lu,%03lu", thousands, remainder);
    } else {
        // For < 1,000 km, show plain number
        snprintf(kmStr, sizeof(kmStr), "%lu", km);
    }
    
    // Use text size 2 for good visibility (12 pixels per character width, 16 pixels height)
    // Text size 2 is more reliable and fits better on 128x64 display
    mainOled->setTextSize(2);
    mainOled->setTextColor(SSD1306_WHITE);
    
    // Calculate text width for centering
    // Text size 2: 12 pixels per character width
    uint8_t numberWidth = strlen(kmStr) * 12;
    uint8_t numberX = (128 - numberWidth) / 2;
    
    // Position at top of screen (y=0)
    // Text size 2 characters are 16 pixels tall, so they fit well
    mainOled->setCursor(numberX, 0);
    mainOled->print(kmStr);
    
    mainOled->display();
}