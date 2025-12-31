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

Adafruit_SSD1306 *fuelDispPtr = nullptr;
Adafruit_SSD1306 *tempDispPtr = nullptr;
// Main OLED (128x64 SSD1309) will be created in setup() after I2C is initialized
Adafruit_SSD1306 *mainOledPtr = nullptr;

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
Displays *displaysPtr = nullptr;

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
// Odometer
// =====================
// TODO: Store odometer in EEPROM/Flash to persist across power cycles
uint32_t odometerMeters = 0; // Total meters traveled (for precision)
unsigned long lastOdometerUpdate = 0;

// =====================
// Setup
// =====================

void setup()
{
  Wire.begin(TEMP_SDA, TEMP_SCL, 400000); // Temp OLED + Main OLED (bus 0)
  delay(50);
  I2C_FUEL.begin(FUEL_SDA, FUEL_SCL, 400000);
  delay(50);

  Serial.begin(115200);
  delay(200);
  Serial.println("Dashboard booting...");

  // Create OLED display objects NOW (after I2C is initialized)
  static Adafruit_SSD1306 fuelDisp(128, 32, &I2C_FUEL, -1);
  static Adafruit_SSD1306 tempDisp(128, 32, &Wire, -1);
  // Main OLED is on bus 1 at 0x3D (I2C scan confirmed this)
  static Adafruit_SSD1306 mainOled(128, 64, &I2C_FUEL, -1); // 128x64 SSD1309 on bus 1
  fuelDispPtr = &fuelDisp;
  tempDispPtr = &tempDisp;
  mainOledPtr = &mainOled;

  // --- Comprehensive I2C bus scan
  Serial.println("\n=== I2C Bus Scanner ===");

  // Scan bus 0 (Temp/Main OLED)
  Serial.println("Scanning I2C bus 0 (Temp/Main OLED on SDA=21, SCL=22)...");
  int nDevices = 0;
  bool found0x3C_bus0 = false;
  bool found0x3D_bus0 = false;

  for (byte address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.printf("  Device found at address 0x%02X", address);
      if (address == 0x3C)
      {
        Serial.print(" (OLED default)");
        found0x3C_bus0 = true;
      }
      else if (address == 0x3D)
      {
        Serial.print(" (OLED alternate)");
        found0x3D_bus0 = true;
      }
      Serial.println();
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.printf("  Unknown error at address 0x%02X\n", address);
    }
  }
  if (nDevices == 0)
  {
    Serial.println("  No I2C devices found on bus 0");
  }
  else
  {
    Serial.printf("  Total devices on bus 0: %d\n", nDevices);
  }

  // Check specific OLED addresses on bus 0
  Serial.println("\nChecking OLED addresses on bus 0:");
  Wire.beginTransmission(0x3C);
  byte error3C = Wire.endTransmission();
  Serial.printf("  0x3C: %s\n", error3C == 0 ? "FOUND" : (error3C == 2 ? "NACK" : "ERROR"));

  Wire.beginTransmission(0x3D);
  byte error3D = Wire.endTransmission();
  Serial.printf("  0x3D: %s\n", error3D == 0 ? "FOUND" : (error3D == 2 ? "NACK" : "ERROR"));

  // Scan bus 1 (Fuel OLED)
  Serial.println("\nScanning I2C bus 1 (Fuel OLED on SDA=18, SCL=19)...");
  nDevices = 0;
  bool found0x3C_bus1 = false;
  bool found0x3D_bus1 = false;

  for (byte address = 1; address < 127; address++)
  {
    I2C_FUEL.beginTransmission(address);
    byte error = I2C_FUEL.endTransmission();
    if (error == 0)
    {
      Serial.printf("  Device found at address 0x%02X", address);
      if (address == 0x3C)
      {
        Serial.print(" (OLED default)");
        found0x3C_bus1 = true;
      }
      else if (address == 0x3D)
      {
        Serial.print(" (OLED alternate)");
        found0x3D_bus1 = true;
      }
      Serial.println();
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.printf("  Unknown error at address 0x%02X\n", address);
    }
  }
  if (nDevices == 0)
  {
    Serial.println("  No I2C devices found on bus 1");
  }
  else
  {
    Serial.printf("  Total devices on bus 1: %d\n", nDevices);
  }

  // Check specific OLED addresses on bus 1
  Serial.println("\nChecking OLED addresses on bus 1:");
  I2C_FUEL.beginTransmission(0x3C);
  byte error3C_bus1 = I2C_FUEL.endTransmission();
  Serial.printf("  0x3C: %s\n", error3C_bus1 == 0 ? "FOUND" : (error3C_bus1 == 2 ? "NACK" : "ERROR"));

  I2C_FUEL.beginTransmission(0x3D);
  byte error3D_bus1 = I2C_FUEL.endTransmission();
  Serial.printf("  0x3D: %s\n", error3D_bus1 == 0 ? "FOUND" : (error3D_bus1 == 2 ? "NACK" : "ERROR"));

  Serial.println("=== End I2C Scanner ===\n");

  // --- OLED init - try to initialize, set flags based on success
  bool fuelOledConnected = false;
  bool tempOledConnected = false;
  bool mainOledConnected = false;

  // Fuel OLED on bus 1
  if (fuelDisp.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    fuelOledConnected = true;
    Serial.println("Fuel OLED initialized at 0x3C on bus 1");
  }
  else
  {
    Serial.println("Fuel OLED not connected - continuing without it");
  }

  // Temp OLED on bus 0 - try 0x3C first
  uint8_t tempOledAddr = 0;
  if (tempDisp.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    tempOledConnected = true;
    tempOledAddr = OLED_ADDR;
    Serial.println("Temp OLED initialized at 0x3C on bus 0");
  }
  else
  {
    Serial.println("Temp OLED not connected - continuing without it");
  }

  // Main OLED (128x64 SSD1309) on bus 1 at 0x3D (I2C scan shows it's on bus 1, not bus 0)
  uint8_t mainOledAddr = 0;
  if (mainOled.begin(SSD1306_SWITCHCAPVCC, 0x3D))
  {
    mainOledConnected = true;
    mainOledAddr = 0x3D;
    Serial.println("Main OLED (128x64) initialized at 0x3D on bus 1");
  }
  else
  {
    Serial.println("Main OLED failed to initialize at 0x3D on bus 1 - continuing without it");
    Serial.printf("  Temp OLED address: 0x%02X on bus 0\n", tempOledAddr);
    Serial.printf("  Fuel OLED address: 0x%02X on bus 1\n", OLED_ADDR);
    mainOledPtr = nullptr;
  }

  // Safety check: ensure addresses are different on the same bus
  if (tempOledConnected && mainOledConnected && tempOledAddr == mainOledAddr)
  {
    Serial.println("ERROR: Temp OLED and Main OLED have the same address! Disabling Main OLED.");
    mainOledConnected = false;
    mainOledPtr = nullptr;
  }
  else if (tempOledConnected && mainOledConnected)
  {
    Serial.printf("OLED addresses confirmed: Temp=0x%02X (bus 0), Main=0x%02X (bus 1)\n", tempOledAddr, mainOledAddr);
  }

  // Create Displays object (after OLEDs are created and initialized)
  static Displays displays(fuelDisp, tempDisp, mainOledPtr);
  displaysPtr = &displays;

  // --- Multiplexers
  speedoMux.begin();
  rpmMux.begin();
  dashMux.begin();

  // --- Output modules
  speedo.begin();
  rpm.begin();
  dash.begin();

  displaysPtr->begin(mainOledConnected, fuelOledConnected, tempOledConnected);

  // --- Sensor modules
  analogs.begin();
  digitalInputs.begin();
  rpmInput.begin();

  if (mainOledConnected)
  {
    displaysPtr->showOdometer(odometerMeters / 1000); // Convert meters to km
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
  // rpm.setRPM(rpmInput.rpm());
  rpm.setRPM(8000);

  // TODO: real speed calculation later
  speedo.setSpeed(100);

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

  // --- Displays (throttle updates to avoid flickering)
  static unsigned long lastDisplayUpdate = 0;
  unsigned long now = millis();
  if (now - lastDisplayUpdate >= 200)
  { // Update displays every 200ms (5Hz)
    lastDisplayUpdate = now;

    displaysPtr->showFuel(analogs.fuelPercent());
    displaysPtr->showTemp(analogs.tempPercent());
  }

  // Update odometer based on speed (integrate speed over time)
  if (lastOdometerUpdate > 0)
  {
    float speedKph = speedInput.speedKph();
    if (speedKph > 0.1f)
    { // Only update if moving (avoid drift when stationary)
      unsigned long deltaMs = now - lastOdometerUpdate;
      float deltaHours = deltaMs / 3600000.0f; // Convert ms to hours
      float deltaKm = speedKph * deltaHours;
      uint32_t deltaMeters = (uint32_t)(deltaKm * 1000.0f); // Convert to meters for precision
      odometerMeters += deltaMeters;
    }
  }
  lastOdometerUpdate = now;

  // Display odometer on main OLED (throttled to avoid flickering)
  static unsigned long lastOdometerDisplay = 0;
  if (now - lastOdometerDisplay >= 500)
  { // Update odometer display every 500ms (2Hz)
    lastOdometerDisplay = now;
    displaysPtr->showOdometer(odometerMeters / 1000);
  }

  // --- Logger (print state every second)
  static unsigned long lastLogMs = 0;
  if (now - lastLogMs >= 1000)
  {
    lastLogMs = now;

    Serial.println("\n=== Dashboard Status ===");
    Serial.printf("Uptime: %lu s\n", now / 1000);

    // Sensors
    Serial.println("\n--- Sensors ---");
    Serial.printf("Temperature: %d°C (%.1f%%)\n", analogs.tempC(), analogs.tempPercent());

    // Fuel debug info
    uint16_t rawFuel = analogRead(PIN_FUEL_ADC);
    // Validate raw reading before calculating voltage
    if (rawFuel > ADC_MAX)
    {
      Serial.printf("Fuel: %.1f%% (Raw ADC: INVALID/%u, ADC_MAX: %u) - CHECK CONNECTIONS!\n",
                    analogs.fuelPercent(), rawFuel, ADC_MAX);
    }
    else if (rawFuel == 0)
    {
      Serial.printf("Fuel: %.1f%% (Raw ADC: 0 - connection glitch, using filtered value)\n",
                    analogs.fuelPercent());
    }
    else
    {
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
    Serial.printf("Fuel OLED: %s | Temp OLED: %s | Main OLED: %s\n",
                  displaysPtr->isFuelOledConnected() ? "OK" : "N/A",
                  displaysPtr->isTempOledConnected() ? "OK" : "N/A",
                  displaysPtr->isMainOledConnected() ? "OK" : "N/A");
    Serial.println("=====================\n");
  }
}