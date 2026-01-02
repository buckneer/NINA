#pragma once
#include <Arduino.h>

// ESP32-C3 SuperMini Hardware Configuration
// SuperMini exposes: GPIO0-10, GPIO20 (RX), GPIO21 (TX)
// IMPORTANT: GPIO2, GPIO8, GPIO9 are STRAPPING PINS - must avoid!
// GPIO0,1 = USB/Serial, GPIO2,8,9 = Strapping, GPIO6,7 = JTAG (can use if JTAG not needed)
// Safe pins: GPIO3,4,5,10,20,21

// Button Inputs
constexpr uint8_t PIN_BUTTON_OK = 3;
constexpr uint8_t PIN_BUTTON_NEXT = 4;

// Speed Sensor
constexpr uint8_t PIN_SPEED_SENSOR = 5;
constexpr uint32_t HALL_PULSES_PER_MILE = 3200;
constexpr float SPEEDO_METERS_PER_PULSE = 1609.344f / HALL_PULSES_PER_MILE;

// I2C Slave
// Using GPIO10 and GPIO20 for I2C (safe pins on SuperMini)
// Note: GPIO6,7 are JTAG but can be used for I2C if JTAG not needed
constexpr uint8_t I2C_SLAVE_ADDRESS = 0x42;
constexpr uint8_t I2C_SDA = 8;
constexpr uint8_t I2C_SCL = 9;