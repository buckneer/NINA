#pragma once
#include <Arduino.h>
#include <AnalogSensors.h> // For TempVPoint definition

// ─────────────────────────────────────────────
// NINABrain – Hardware Configuration
// Single source of truth
// ─────────────────────────────────────────────

// =================================================
// ===== ANALOG INPUTS (ADC1 – WiFi safe)
// =================================================

constexpr uint8_t PIN_TEMP_ADC = 35; // Engine temp sensor (divider)
constexpr uint8_t PIN_FUEL_ADC = 32; // Fuel sender (divider)

// =================================================
// ===== DIGITAL INPUTS (optocouplers, ACTIVE LOW)
// =================================================

constexpr uint8_t PIN_RPM = 34; // Ignition / coil pulse (input only)
constexpr uint8_t PIN_BRAKE = 25;
constexpr uint8_t PIN_OIL = 26;
constexpr uint8_t PIN_INDICATORS = 27;
constexpr uint8_t PIN_HIGH_BEAM = 14;
constexpr uint8_t PIN_LIGHTS = 12; // ⚠ boot strap pin
constexpr uint8_t PIN_FOG = 13;
constexpr uint8_t PIN_BATTERY = 33; // Alternator D+

// Optional / future
constexpr uint8_t PIN_HALL = 39;

// =================================================
// ===== I2C BUSES
// =================================================

// Temp OLED + LCD (I2C bus 0)
constexpr uint8_t TEMP_SDA = 21;
constexpr uint8_t TEMP_SCL = 22;

// Fuel OLED (I2C bus 1)
constexpr uint8_t FUEL_SDA = 18;
constexpr uint8_t FUEL_SCL = 19;

// I2C device addresses
constexpr uint8_t OLED_ADDR = 0x3C;

// =================================================
// ===== SHIFT REGISTER OUTPUTS (shared SRCLK)
// =================================================

constexpr uint8_t SRCLK = 23;

// ---- RPM BAR (4 × 74HC595)
constexpr uint8_t RPM_DATA = 2;	  // ⚠ boot pin
constexpr uint8_t RPM_LATCH = 15; // ⚠ boot pin

// ---- DASH WARNING LIGHTS (1 × 74HC595)
constexpr uint8_t DASH_DATA = 5;
constexpr uint8_t DASH_LATCH = 17;

// ---- SPEEDOMETER (2 × 74HC595)
constexpr uint8_t SPD_DATA = 16;
constexpr uint8_t SPD_LATCH = 4;

// =================================================
// ===== MULTIPLEX TIMING (milliseconds)
// =================================================
// attach_ms() is used (ESP32 core Ticker)

constexpr uint32_t RPM_MUX_MS = 1;	// fast bar refresh
constexpr uint32_t SPD_MUX_MS = 1;	// digits
constexpr uint32_t DASH_MUX_MS = 2; // warning lights

// =================================================
// ===== FUEL CALIBRATION
// =================================================

constexpr uint8_t LOW_FUEL_THRESHOLD = 20; // %
constexpr float FUEL_ADC_V_MIN = 0.40f;	   // empty
constexpr float FUEL_ADC_V_MAX = 1.80f;	   // full

// =================================================
// ===== RPM CALCULATION
// =================================================

// pulses per engine revolution (adjust to ignition type)
constexpr uint8_t RPM_PULSES_PER_REV = 2;

// measurement window (ms)
constexpr uint16_t RPM_SAMPLE_MS = 200;

// =================================================
// ===== ADC SETTINGS
// =================================================

constexpr uint8_t ADC_BITS = 12;
constexpr float ADC_REF_V = 3.3f;
constexpr uint16_t ADC_MAX = (1 << ADC_BITS) - 1;

// =================================================
// ===== ADC INPUT DIVIDER (shared by analog inputs)
// Vin -> R1 -> ADC -> R2 -> GND
// =================================================

constexpr float ADC_DIV_R1 = 6800.0f; // ohms (top resistor)
constexpr float ADC_DIV_R2 = 2000.0f; // ohms (bottom resistor)

constexpr float ADC_DIV_RATIO =
	ADC_DIV_R2 / (ADC_DIV_R1 + ADC_DIV_R2); // ≈ 0.22727

constexpr float ADC_DIV_INV =
	(ADC_DIV_R1 + ADC_DIV_R2) / ADC_DIV_R2; // ≈ 4.4

// =================================================
// ===== YUGO TEMP SENSOR (ADC-voltage LUT)
// =================================================
// Note: TempVPoint is defined in AnalogSensors.h

// ORDER: cold → hot (voltage usually rises with temp)
constexpr TempVPoint TEMP_V_TABLE[] = {
	{0.35f, 20},
	{0.55f, 40},
	{0.80f, 60},
	{1.05f, 80},
	{1.30f, 100},
	{1.50f, 110}};

constexpr size_t TEMP_V_TABLE_SIZE =
	sizeof(TEMP_V_TABLE) / sizeof(TEMP_V_TABLE[0]);

constexpr int TEMP_MIN_C = 0;
constexpr int TEMP_MAX_C = 120;

// ======================
// SPEEDO CALIBRATION
// ======================

constexpr float METERS_PER_MILE = 1609.344f;

// 1600 rotations per mile × 2 poles (CHANGE)
constexpr uint32_t HALL_PULSES_PER_MILE = 3200;

constexpr float SPEEDO_METERS_PER_PULSE =
	METERS_PER_MILE / HALL_PULSES_PER_MILE; // ≈ 0.503 m