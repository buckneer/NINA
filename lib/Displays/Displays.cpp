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
  hd44780_I2Cexp& lcdDisplay
)
  : fuel(fuelDisplay),
    temp(tempDisplay),
    lcd(lcdDisplay) {}

void Displays::begin() {
    fuel.clearDisplay();
    fuel.display();

    temp.clearDisplay();
    temp.display();

    int status = lcd.begin(16, 2);
    if (status == 0) {
        lcd.backlight();
        lcd.clear();
    } else {
        Serial.printf("LCD init failed: %d\n", status);
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
    drawBar(fuel, pct);
}

void Displays::showTemp(uint8_t pct) {
    drawBar(temp, pct);
}

void Displays::showLCD(const char* line1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
}

void Displays::showLCD(const char* line1, const char* line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}