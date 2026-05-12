# I2C Protocol Specification

## Overview

This document describes the I2C communication protocol used between the NINA controller boards and display boards.

## Bus Configuration

### Bus 0 (Primary)
- **SDA**: GPIO 21 (ESP32)
- **SCL**: GPIO 22 (ESP32)
- **Speed**: 400kHz (Fast Mode)
- **Devices**:
  - Temperature OLED Display (0x3C)
  - LCD Display (0x27)

### Bus 1 (Secondary)
- **SDA**: GPIO 18 (ESP32)
- **SCL**: GPIO 19 (ESP32)
- **Speed**: 100kHz (Standard Mode, for Pro Mini compatibility)
- **Devices**:
  - Fuel OLED Display (0x3C)
  - Main OLED Display (0x3D)
  - RP2040 (Pico-class) I2C Slave (0x42) — buttons

## Device Addresses

| Device | Address | Bus | Notes |
|--------|---------|-----|-------|
| Temperature OLED | 0x3C | 0 | SSD1306 128×32 |
| Fuel OLED | 0x3C | 1 | SSD1306 128×32 |
| Main OLED | 0x3D | 1 | SSD1306 128×64 |
| LCD Display | 0x27 | 0 | HD44780 with I2C backpack |
| RP2040 I2C Slave | 0x42 | 1 | Buttons (NEXT/OK), speedometer (hall) |

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

## RP2040 (Pico-class) to ESP32 Communication (I2C)

The RP2040 board acts as an **I2C slave** on **Bus 1** at address **0x42**.
The ESP32 (master) polls the RP2040 at 20 Hz with an `I2C read` to fetch
buttons, speed, and battery data.

### Packet Format (ESP32 reads from 0x42)

ESP32 requests **5 bytes** from the RP2040:

```
Byte 0 : Buttons bitmask (uint8_t)
           bit0 = NEXT pressed  (active-low, pulled to GND, GPIO16)
           bit1 = OK   pressed  (active-low, pulled to GND, GPIO17)
           bit2-7 = reserved

Byte 1 : ~Byte 0  (bitwise NOT – integrity check)

Byte 2 : speed_lo  (uint8_t, LSB of uint16_t LE)
Byte 3 : speed_hi  (uint8_t, MSB of uint16_t LE)
           Speed = (Byte2 | Byte3<<8) / 10.0  [km/h, 0.1 km/h resolution]
           Source: hall effect sensor on GPIO14

Byte 4 : XOR checksum = Byte0 ^ Byte1 ^ Byte2 ^ Byte3
```

**Validation (ESP32 side):**
- `(byte0 ^ byte1) == 0xFF` — buttons integrity
- `byte0 ^ byte1 ^ byte2 ^ byte3 ^ byte4 == 0` — full packet XOR

If either check fails the packet is discarded and the ESP32 retries on the next poll.

## Future Enhancements

- Multi-master support
- Error recovery mechanisms
- Device discovery protocol
- Status reporting from display boards
- Bidirectional communication (ESP32 → ESP32-C3 commands)
- Encryption support for ESP-NOW
- Multiple ESP32-C3 slaves support

