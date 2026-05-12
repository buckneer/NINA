#pragma once
#include <Arduino.h>

// ─────────────────────────────────────────────
// NINA RP2040 – Hardware Configuration
// ─────────────────────────────────────────────

constexpr uint32_t SERIAL_BAUD = 115200;

// Buttons (active-low: press pulls pin to GND, internal pull-up enabled)
constexpr uint8_t  PIN_BTN_NEXT       = 16;
constexpr uint8_t  PIN_BTN_OK         = 17;
constexpr bool     BUTTON_ACTIVE_LOW  = true;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 25;

// Hall effect sensor (speedometer)
constexpr uint8_t  PIN_HALL               = 14;
constexpr float    METERS_PER_MILE        = 1609.344f;
constexpr uint32_t HALL_PULSES_PER_MILE   = 3200; // 1600 rotations × 2 poles
constexpr float    SPEEDO_METERS_PER_PULSE = METERS_PER_MILE / HALL_PULSES_PER_MILE;
constexpr uint16_t SPEEDO_SAMPLE_MS        = 200;

// I2C slave (Mbed core defaults: GP4=SDA, GP5=SCL)
constexpr uint8_t  PIN_I2C_SDA    = 4;
constexpr uint8_t  PIN_I2C_SCL    = 5;
constexpr uint32_t I2C_FREQ_HZ    = 100000;
constexpr uint8_t  I2C_SLAVE_ADDR = 0x42;
