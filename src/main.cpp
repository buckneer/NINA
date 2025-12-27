#include <Arduino.h>

// =====================
// Hardware config
// =====================
#include "HardwareConfig.h"

// =====================
// Core infrastructure
// =====================
#include <Multiplex.h>

// =====================
// Output modules
// =====================
#include <Speedo.h>
#include <RPM.h>
#include <DashLights.h>
#include <Displays.h>

// =====================
// Sensor modules
// =====================
#include <AnalogSensors.h>
#include <DigitalInputs.h>
#include <RPMInput.h>
#include <SpeedInput.h>

// =====================
// I2C buses
// =====================

TwoWire I2C_TEMP(0); // Temp OLED + LCD
TwoWire I2C_FUEL(1); // Fuel OLED

// =====================
// Display devices
// =====================

Adafruit_SSD1306 fuelDisp(128, 32, &I2C_FUEL, -1);
Adafruit_SSD1306 tempDisp(128, 32, &I2C_TEMP, -1);
hd44780_I2Cexp lcd;

// =====================
// Multiplexers
// =====================

// Speedo: 2 × 74HC595 → 3 digits
Multiplex<2> speedoMux(
    SPD_DATA,
    SRCLK,
    SPD_LATCH,
    3,
    SPD_MUX_MS);

// RPM bar: 4 × 74HC595
Multiplex<4> rpmMux(
    RPM_DATA,
    SRCLK,
    RPM_LATCH,
    RPMMeter::CHANNELS,
    RPM_MUX_MS);

// Dash warning lights: 1 × 74HC595
Multiplex<1> dashMux(
    DASH_DATA,
    SRCLK,
    DASH_LATCH,
    1,
    DASH_MUX_MS);

// =====================
// High-level output modules
// =====================

Speedo speedo(speedoMux);
RPMMeter rpm(rpmMux);
DashLights dash(dashMux);

Displays displays(
    fuelDisp,
    tempDisp,
    lcd);

// =====================
// Sensor wiring (pin injection)
// =====================

AnalogSensors::Pins analogPins{
    .temp = PIN_TEMP_ADC,
    .fuel = PIN_FUEL_ADC};

AnalogSensorsConfig analogConfig{
    .adcBits = ADC_BITS,
    .adcRefV = ADC_REF_V,
    .adcMax = ADC_MAX,
    .tempVTable = TEMP_V_TABLE,
    .tempVTableSize = TEMP_V_TABLE_SIZE,
    .tempMinC = TEMP_MIN_C,
    .tempMaxC = TEMP_MAX_C,
    .fuelAdcVMin = FUEL_ADC_V_MIN,
    .fuelAdcVMax = FUEL_ADC_V_MAX};

AnalogSensors analogs(analogPins, analogConfig);

DigitalInputs::Pins digitalPins{
    PIN_BRAKE,
    PIN_OIL,
    PIN_INDICATORS,
    PIN_HIGH_BEAM,
    PIN_LIGHTS,
    PIN_FOG,
    PIN_BATTERY};

DigitalInputs digitalInputs(digitalPins);

RPMInput rpmInput(PIN_RPM, RPM_SAMPLE_MS, RPM_PULSES_PER_REV);

// =====================
// SpeedoInput
// =====================

SpeedInput speedInput(PIN_HALL, SPEEDO_METERS_PER_PULSE);

// =====================
// Setup
// =====================

void setup()
{
  Serial.begin(115200);
  delay(200);

  Serial.println("Dashboard booting...");

  // --- I2C init
  I2C_TEMP.begin(TEMP_SDA, TEMP_SCL, 400000);
  I2C_FUEL.begin(FUEL_SDA, FUEL_SCL, 400000);

  // --- OLED init
  if (!fuelDisp.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("Fuel OLED init failed");
  }
  if (!tempDisp.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("Temp OLED init failed");
  }

  // --- Multiplexers
  speedoMux.begin();
  rpmMux.begin();
  dashMux.begin();

  // --- Output modules
  speedo.begin();
  rpm.begin();
  dash.begin();
  displays.begin();

  // --- Sensor modules
  analogs.begin();
  digitalInputs.begin();
  rpmInput.begin();

  displays.showLCD("DASH READY", "SENSORS ONLINE");
  Serial.println("Setup complete");
}

// =====================
// Loop
// =====================

void loop()
{
  // --- Read sensors
  analogs.update();
  digitalInputs.update();
  rpmInput.update();

  // --- Outputs
  rpm.setRPM(rpmInput.rpm());

  // TODO: real speed calculation later
  speedo.setSpeed(0);

  // --- Dash lights
  dash.setBrakes(digitalInputs.brake());
  dash.setOil(digitalInputs.oil());
  dash.setIndicators(digitalInputs.indicators());
  dash.setHighBeam(digitalInputs.highBeam());
  dash.setHeadlights(digitalInputs.lights());
  dash.setFogLights(digitalInputs.fog());
  dash.setBattery(digitalInputs.battery());

  // --- Displays
  displays.showFuel(analogs.fuelPercent());
  displays.showTemp(analogs.tempPercent());
}