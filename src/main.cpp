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

TwoWire I2C_FUEL(1); // Fuel OLED

// =====================
// Display devices - will be initialized in setup() after I2C is ready
// =====================

Adafruit_SSD1306* fuelDispPtr = nullptr;
Adafruit_SSD1306* tempDispPtr = nullptr;
// LCD object will be created in setup() after I2C is initialized
LiquidCrystal_I2C* lcdPtr = nullptr;

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

// Displays object will be created in setup() after LCD detection
Displays* displaysPtr = nullptr;

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
  Wire.begin(TEMP_SDA, TEMP_SCL, 400000); // Temp OLED + LCD (bus 0)
  delay(50);
  I2C_FUEL.begin(FUEL_SDA, FUEL_SCL, 400000);
  delay(50);
  
  Serial.begin(115200);
  delay(200);
  Serial.println("Dashboard booting...");
  
  // Create OLED display objects NOW (after I2C is initialized)
  static Adafruit_SSD1306 fuelDisp(128, 32, &I2C_FUEL, -1);
  static Adafruit_SSD1306 tempDisp(128, 32, &Wire, -1);
  fuelDispPtr = &fuelDisp;
  tempDispPtr = &tempDisp;

  // --- OLED init - try to initialize, set flags based on success
  bool fuelOledConnected = false;
  bool tempOledConnected = false;
  
  if (fuelDisp.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    fuelOledConnected = true;
    Serial.println("Fuel OLED initialized");
  } else {
    Serial.println("Fuel OLED not connected - continuing without it");
  }
  
  if (tempDisp.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    tempOledConnected = true;
    Serial.println("Temp OLED initialized");
  } else {
    Serial.println("Temp OLED not connected - continuing without it");
  }
  
  // LCD initialization
  bool lcdConnected = false;
  // Create LCD object (using Wire default instance, same bus 0 as Temp OLED)
  // LCD address, columns, rows
  static LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
  lcdPtr = &lcd;
  delay(50);
  
  // Try to initialize LCD
  lcd.init();
  delay(100);
  lcd.backlight();
  delay(50);
  lcd.clear();
  lcdConnected = true;
  Serial.println("LCD initialized");
  
  // Create Displays object (after OLEDs and LCD are created and initialized)
  static Displays displays(fuelDisp, tempDisp, lcdPtr);
  displaysPtr = &displays;

  // --- Multiplexers
  speedoMux.begin();
  rpmMux.begin();
  dashMux.begin();

  // --- Output modules
  speedo.begin();
  rpm.begin();
  dash.begin();
  
  displaysPtr->begin(lcdConnected, fuelOledConnected, tempOledConnected);

  // --- Sensor modules
  analogs.begin();
  digitalInputs.begin();
  rpmInput.begin();

  if (lcdConnected) {
    displaysPtr->showLCD("DASH READY", "SENSORS ONLINE");
  }
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
  // Low fuel warning (below 20%)
  dash.setLowFuel(analogs.fuelPercent() < LOW_FUEL_THRESHOLD);

  // --- Displays
  displaysPtr->showFuel(analogs.fuelPercent());
  displaysPtr->showTemp(analogs.tempPercent());

  // --- Logger (print state every second)
  static unsigned long lastLogMs = 0;
  unsigned long now = millis();
  if (now - lastLogMs >= 1000) {
    lastLogMs = now;
    
    Serial.println("\n=== Dashboard Status ===");
    Serial.printf("Uptime: %lu s\n", now / 1000);
    
    // Sensors
    Serial.println("\n--- Sensors ---");
    Serial.printf("Temperature: %d°C (%.1f%%)\n", analogs.tempC(), analogs.tempPercent());
    
    // Fuel debug info
    uint16_t rawFuel = analogRead(PIN_FUEL_ADC);
    // Validate raw reading before calculating voltage
    if (rawFuel > ADC_MAX) {
      Serial.printf("Fuel: %.1f%% (Raw ADC: INVALID/%u, ADC_MAX: %u) - CHECK CONNECTIONS!\n", 
        analogs.fuelPercent(), rawFuel, ADC_MAX);
    } else if (rawFuel == 0) {
      Serial.printf("Fuel: %.1f%% (Raw ADC: 0 - connection glitch, using filtered value)\n", 
        analogs.fuelPercent());
    } else {
      float fuelVoltage = (rawFuel / (float)ADC_MAX) * ADC_REF_V;
      Serial.printf("Fuel: %.1f%% (Raw ADC: %u, Voltage: %.3fV, Min: %.2fV, Max: %.2fV)\n", 
        analogs.fuelPercent(), rawFuel, fuelVoltage, FUEL_ADC_V_MIN, FUEL_ADC_V_MAX);
    }
    
    Serial.printf("RPM: %u\n", rpmInput.rpm());
    Serial.printf("Speed: %.1f km/h\n", speedInput.speedKph());
    
    // Digital inputs
    Serial.println("\n--- Digital Inputs ---");
    Serial.printf("Brake: %s | Oil: %s | Indicators: %s | High Beam: %s\n",
      digitalInputs.brake() ? "ON" : "OFF",
      digitalInputs.oil() ? "ON" : "OFF",
      digitalInputs.indicators() ? "ON" : "OFF",
      digitalInputs.highBeam() ? "ON" : "OFF");
    Serial.printf("Lights: %s | Fog: %s | Battery: %s\n",
      digitalInputs.lights() ? "ON" : "OFF",
      digitalInputs.fog() ? "ON" : "OFF",
      digitalInputs.battery() ? "ON" : "OFF");
    
    // Display status
    Serial.println("\n--- Displays ---");
    Serial.printf("Fuel OLED: %s | Temp OLED: %s | LCD: %s\n",
      displaysPtr->isFuelOledConnected() ? "OK" : "N/A",
      displaysPtr->isTempOledConnected() ? "OK" : "N/A",
      displaysPtr->isLCDConnected() ? "OK" : "N/A");
    Serial.println("=====================\n");
  }
}