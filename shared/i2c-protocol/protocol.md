# I2C Protocol Specification

## Overview

This document describes the I2C communication protocol used between the NINA controller boards and display boards.

## Bus Configuration

### Bus 0 (Primary)
- **SDA**: GPIO 21 (ESP32) / TBD (ESP32-C3)
- **SCL**: GPIO 22 (ESP32) / TBD (ESP32-C3)
- **Speed**: 400kHz (Fast Mode)
- **Devices**:
  - Temperature OLED Display (0x3C)
  - LCD Display (0x27)

### Bus 1 (Secondary)
- **SDA**: GPIO 18 (ESP32) / TBD (ESP32-C3)
- **SCL**: GPIO 19 (ESP32) / TBD (ESP32-C3)
- **Speed**: 400kHz (Fast Mode)
- **Devices**:
  - Fuel OLED Display (0x3C)
  - Main OLED Display (0x3D)

## Device Addresses

| Device | Address | Bus | Notes |
|--------|---------|-----|-------|
| Temperature OLED | 0x3C | 0 | SSD1306 128×32 |
| Fuel OLED | 0x3C | 1 | SSD1306 128×32 |
| Main OLED | 0x3D | 1 | SSD1306 128×64 |
| LCD Display | 0x27 | 0 | HD44780 with I2C backpack |

## Communication Protocol

### Display Updates

All displays use standard I2C communication following their respective device protocols:
- **SSD1306 OLED**: Uses Adafruit SSD1306 library protocol
- **HD44780 LCD**: Uses I2C backpack protocol (typically PCF8574)

### Data Format

Display data is sent as standard I2C transactions:
- Start condition
- Device address (7-bit) + R/W bit
- Register/command bytes
- Data bytes
- Stop condition

## Error Handling

- Devices should be checked for presence before initialization
- Failed I2C transactions should be logged but not block system operation
- Missing displays should be gracefully handled (system continues without them)

## ESP32-C3 to ESP32 Communication

The ESP32-C3 acts as an I2C slave device, sending button states and speed data to the main ESP32 board.

### ESP32-C3 I2C Slave Configuration

- **I2C Address**: 0x42
- **SDA**: GPIO 6 (ESP32-C3)
- **SCL**: GPIO 7 (ESP32-C3)
- **Speed**: 400kHz (Fast Mode)
- **Role**: I2C Slave (responds to requests from ESP32 master)

### Data Packet Format

The ESP32-C3 sends a 5-byte data packet when requested by the ESP32 master:

```
Byte 0: Button States (bitfield)
  Bit 0: Button OK (1 = pressed, 0 = released)
  Bit 1: Button Next (1 = pressed, 0 = released)
  Bits 2-7: Reserved (set to 0)

Byte 1-2: Speed (uint16_t, little-endian)
  Speed in km/h × 10 (e.g., 125 = 12.5 km/h)
  Range: 0-6553.5 km/h

Byte 3: Checksum
  Inverted sum of bytes 0-2
```

### Communication Flow

1. ESP32 master requests data from ESP32-C3 (I2C address 0x42)
2. ESP32-C3 responds with 5-byte data packet
3. ESP32 master validates checksum and processes data
4. Process repeats at regular intervals (recommended: 10-50ms)

### Example Usage (ESP32 Master Side)

```cpp
// On ESP32 master board
Wire.beginTransmission(0x42);
Wire.requestFrom(0x42, 5);
if (Wire.available() >= 5) {
  uint8_t buttons = Wire.read();
  uint16_t speed_x10 = Wire.read() | (Wire.read() << 8);
  uint8_t checksum = Wire.read();
  
  // Validate checksum
  uint8_t calc_checksum = ~(buttons + (speed_x10 & 0xFF) + (speed_x10 >> 8));
  if (calc_checksum == checksum) {
    bool buttonOk = (buttons & 0x01) != 0;
    bool buttonNext = (buttons & 0x02) != 0;
    float speedKph = speed_x10 / 10.0f;
    
    // Use the data...
  }
}
```

## Future Enhancements

- Multi-master support
- Error recovery mechanisms
- Device discovery protocol
- Status reporting from display boards
- Bidirectional communication (ESP32 → ESP32-C3 commands)

