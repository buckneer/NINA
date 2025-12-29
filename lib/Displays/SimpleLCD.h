//
// Simple LCD interface - no external library dependency
// Just a stub that can be implemented later with any LCD library
//

#ifndef NINA_SIMPLELCD_H
#define NINA_SIMPLELCD_H

#include <Arduino.h>

class SimpleLCD {
public:
    SimpleLCD(uint8_t address, uint8_t cols, uint8_t rows) 
        : _address(address), _cols(cols), _rows(rows), _initialized(false) {}
    
    void init() { _initialized = true; }
    void backlight() {}
    void clear() {}
    void setCursor(uint8_t col, uint8_t row) {}
    void print(const char* text) {}
    
    bool isInitialized() const { return _initialized; }

private:
    uint8_t _address;
    uint8_t _cols;
    uint8_t _rows;
    bool _initialized;
};

#endif // NINA_SIMPLELCD_H
