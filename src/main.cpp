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
// WiFi & Network
// =====================
#include <ESP_WiFiManager.h>
#include <ArduinoOTA.h>
#include <WebServer.h>

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
// WiFi Manager & OTA
// =====================
ESP_WiFiManager* esp_wifiManager = nullptr;
WebServer* webServer = nullptr;

// OTA Configuration
const char* OTA_HOSTNAME = "NINA-Dashboard";
const uint16_t OTA_PORT = 3232;
const char* OTA_PASSWORD = "NINA_OTA_2024";

bool otaInitialized = false;

// =====================
// Forward declarations
// =====================
void setupOTA();
void setupWebServer();

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
  delay(500);  // Give Serial time to initialize
  Serial.println("\n\n=== NINA Dashboard Booting ===");
  
  // Initialize dashboard hardware first
  Serial.println("Initializing hardware...");

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
  
  Serial.println("Hardware setup complete");
  
  // --- WiFi Setup using ESP_WiFiManager (after hardware is ready)
  Serial.println("\n=== WiFi Setup ===");
  
  // Create WiFiManager instance now (after Serial is ready)
  esp_wifiManager = new ESP_WiFiManager("NINA-Dashboard-Config");
  webServer = new WebServer(80);
  
  // Set WiFi to STA mode only (station mode - client mode, not access point)
  // This ensures AP is not visible while connecting
  WiFi.mode(WIFI_STA);
  
  // Configure WiFiManager
  esp_wifiManager->setConfigPortalTimeout(180);  // 3 minutes for config portal
  esp_wifiManager->setConnectTimeout(20);  // Try connecting for max 20 seconds before giving up
  esp_wifiManager->setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
  esp_wifiManager->setMinimumSignalQuality(-1);  // -1 = no minimum
  
  Serial.println("Attempting WiFi connection...");
  Serial.println("(No AP visible during connection attempt)");
  
  // Try to connect - autoConnect will:
  // 1. Check for saved credentials
  // 2. Try to connect with saved credentials (timeout after 20 seconds)
  // 3. Only if fails or no credentials, start config portal automatically (AP mode)
  bool connected = esp_wifiManager->autoConnect("NINA-Dashboard-Config");
  
  if (!connected) {
    // Connection failed - config portal is now active (AP mode is automatically enabled)
    Serial.println("\n!!! Config Portal is NOW ACTIVE !!!");
    Serial.println("========================================");
    Serial.println("WiFi SSID: NINA-Dashboard-Config");
    Serial.println("IP Address: 192.168.4.1");
    Serial.println("========================================");
    Serial.println("Connect to the WiFi network above");
    Serial.println("Then open http://192.168.4.1 in your browser");
    Serial.println("(Config portal will timeout in 3 minutes)");
  } else {
    // Connected successfully - ensure we're in STA mode only (disable AP)
    Serial.println("\n✓ WiFi connected successfully!");
    WiFi.mode(WIFI_STA);  // Ensure STA mode only (no AP visible)
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.println("(AP disabled - device not visible in WiFi list)");
    WiFi.setHostname(OTA_HOSTNAME);
    
    // Setup OTA
    setupOTA();
    setupWebServer();
  }
  
  Serial.println("\n=== Setup Complete ===");
}

// =====================
// Loop
// =====================

void setupOTA() {
  if (otaInitialized) return;
  
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  
  ArduinoOTA.onStart([]() {
    Serial.println("OTA Update Started");
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA Update Finished");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  
  ArduinoOTA.begin();
  otaInitialized = true;
  Serial.println("OTA Ready");
}

void setupWebServer() {
  if (!webServer) return;
  
  webServer->on("/", []() {
    String html = "<!DOCTYPE html><html><head><title>NINA Dashboard</title></head><body>";
    html += "<h1>NINA Dashboard</h1>";
    html += "<p>WiFi Connected!</p>";
    html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
    html += "</body></html>";
    webServer->send(200, "text/html", html);
  });
  
  webServer->begin();
  Serial.println("Web Server Started");
}

void loop()
{
  // Handle WiFiManager (in case config portal is active)
  // This needs to be called regularly for config portal to work
  if (esp_wifiManager && WiFi.status() != WL_CONNECTED) {
    // Config portal might be active, WiFiManager handles it internally
    // Check if connection just happened from config portal
    static bool portalWasActive = false;
    portalWasActive = true;
    
    // Check periodically if we just got connected
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 1000) {
      lastCheck = millis();
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected from config portal!");
        // Disable AP mode - switch to STA only so AP is not visible
        WiFi.mode(WIFI_STA);
        Serial.println("AP disabled - device not visible in WiFi list");
        WiFi.setHostname(OTA_HOSTNAME);
        setupOTA();
        setupWebServer();
        portalWasActive = false;
      }
    }
  }
  
  // Handle OTA updates (if WiFi connected)
  if (WiFi.status() == WL_CONNECTED) {
    // Ensure we stay in STA mode only (no AP visible)
    if (WiFi.getMode() != WIFI_STA) {
      WiFi.mode(WIFI_STA);
    }
    
    ArduinoOTA.handle();
    if (webServer) {
      webServer->handleClient();
    }
  }
  
  // --- Read sensors
  analogs.update();
  digitalInputs.update();
  rpmInput.update();

  // --- Outputs
  rpm.setRPM(rpmInput.rpm());
  // rpm.setRPM(8000);

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

    // dash.setBrakes(true);
  // dash.setOil(true);
  // dash.setIndicators(true);
  // dash.setHighBeam(true);
  // dash.setHeadlights(true);
  // dash.setFogLights(true);
  // dash.setBattery(true);
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
    
    // WiFi status
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("WiFi: Connected (IP: %s, RSSI: %d dBm)\n", 
                    WiFi.localIP().toString().c_str(), WiFi.RSSI());
    } else {
      Serial.println("WiFi: Disconnected");
    }

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