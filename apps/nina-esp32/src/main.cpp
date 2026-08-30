#include <Arduino.h>
#include <esp_sleep.h>

#include "HardwareConfig.h"

#include <Multiplex.h>
#include <Speedo.h>
#include <RPM.h>
#include <DashLights.h>
#include <Dashboard.h>
#include <AnalogSensors.h>
#include <DigitalInputs.h>
#include <RPMInput.h>
#include <PicoLink.h>
#include <Odometer.h>
#include <I2CScanner.h>
#include <UINav.h>
#include <UIManager.h>
#include <DashScreen.h>
#include <ClockWidget.h>
#include <MenuScreen.h>
#include <ClockSetScreen.h>
#include <ClockService.h>
#include <OTAScreen.h>
#include <WiFiManager.h>
#include <InfoScreen.h>

static constexpr float RPM_MAX_RISE_PER_SEC = 12000.0f;
static constexpr float RPM_MAX_FALL_PER_SEC = 16000.0f;

static float displayedRPM = 0.0f;
static uint32_t lastRpmSlewUs = 0;

// ============================================================================
// I2C
// ============================================================================

TwoWire I2C_FUEL(1);

// ============================================================================
// Multiplexers
// ============================================================================

Multiplex<2> speedoMux(
    SPD_DATA,
    SRCLK,
    SPD_LATCH,
    3,
    SPD_MUX_MS);

Multiplex<4> rpmMux(
    RPM_DATA,
    SRCLK,
    RPM_LATCH,
    RPMMeter::CHANNELS,
    RPM_MUX_MS);

Multiplex<1> dashMux(
    DASH_DATA,
    SRCLK,
    DASH_LATCH,
    1,
    DASH_MUX_MS);

// ============================================================================
// Dashboard components
// ============================================================================

Speedo speedo(speedoMux);

RPMMeter rpm(
    rpmMux,
    RPM_MAX);

DashLights dash(dashMux);

Dashboard dashboard;

// ============================================================================
// Analog sensors
// ============================================================================

AnalogSensors::Pins analogPins{
    .temp = PIN_TEMP_ADC,
    .fuel = PIN_FUEL_ADC};

AnalogSensorsConfig analogConfig{
    .adcBits = ADC_BITS,
    .adcMax = ADC_MAX,

    .senderSupplyV = SENDER_SUPPLY_V,

    .tempPullupOhms = TEMP_PULLUP_OHMS,
    .fuelPullupOhms = FUEL_PULLUP_OHMS,

    .tempRTable = TEMP_R_TABLE,
    .tempRTableSize = TEMP_R_TABLE_SIZE,

    .fuelTable = FUEL_TABLE,
    .fuelTableSize = FUEL_TABLE_SIZE,

    .tempMinC = TEMP_MIN_C,
    .tempMaxC = TEMP_MAX_C,
};

AnalogSensors analogs(
    analogPins,
    analogConfig);

// ============================================================================
// Digital inputs
// ============================================================================

DigitalInputs::Pins digitalPins{
    PIN_BRAKE,
    PIN_OIL,
    PIN_INDICATORS,
    PIN_HIGH_BEAM,
    PIN_LIGHTS,
    PIN_FOG,
    PIN_BATTERY,
};

DigitalInputs digitalInputs(
    digitalPins);

// ============================================================================
// RPM input
// ============================================================================

RPMInput rpmInput(
    PIN_RPM,
    RPM_PULSES_PER_REV);

// ============================================================================
// Pico / odometer
// ============================================================================

PicoLink pico;
Odometer odometer(SPEEDO_METERS_PER_PULSE);

// ============================================================================
// UI
// ============================================================================

UIManager ui;
DashScreen dashScreen;
ClockWidget clockWidget;
MenuScreen settingsMenu;
ClockSetScreen clockSet;
InfoScreen infoScreen;

WiFiManager wifiManager;
OTAService otaService;

OTAScreen otaScreen(
    wifiManager,
    otaService);

// ============================================================================
// Power
// ============================================================================

static void enterDeepSleep(bool saveOdometer)
{
    if (saveOdometer)
    {
        odometer.save();
    }

    esp_sleep_enable_ext0_wakeup(
        GPIO_NUM_33,
        0); // wake on LOW (IGN on)

    Serial.println(
        "[Power] IGN off — entering deep sleep");

    Serial.flush();
    esp_deep_sleep_start();
}

// ============================================================================
// Setup
// ============================================================================

void setup()
{
    // Hardware pull-up via R3.
    pinMode(
        PIN_IGN_SLEEP,
        INPUT);

    if (digitalRead(PIN_IGN_SLEEP) == HIGH)
    {
        // Dashboard has only just booted.
        // Odometer NVS has not been opened yet, so nothing needs saving.
        enterDeepSleep(false);
    }
    // ======================
    // Multiplexers
    // ======================

    speedoMux.begin();
    rpmMux.begin();
    dashMux.begin();

    // ======================
    // LED outputs
    // ======================

    speedo.begin();
    rpm.begin();
    dash.begin();

    speedo.setSpeed(0);
    rpm.setRPM(0);

    // ======================
    // Serial
    // ======================

    Serial.begin(115200);
    ClockService::initializeIfNeeded();

    // ======================
    // I2C
    // ======================

    Wire.begin(
        TEMP_SDA,
        TEMP_SCL,
        400000);

    I2C_FUEL.begin(
        FUEL_SDA,
        FUEL_SCL,
        400000);

    I2CScanner::scan(
        Wire,
        "bus0 (Wire)");

    I2CScanner::scan(
        I2C_FUEL,
        "bus1 (I2C_FUEL)");

    // ======================
    // Modules
    // ======================

    pico.begin(
        Wire,
        I2C_FUEL);

    dashboard.begin(
        Wire,
        I2C_FUEL);

    analogs.begin();
    digitalInputs.begin();
    rpmInput.begin();

    odometer.begin();

    // ======================
    // UI
    // ======================

    dashScreen.setWidget(
        &clockWidget);

    settingsMenu.addItem(
        {"Set Clock",
         nullptr,
         &clockSet});

    settingsMenu.addItem(
        {"Trip Reset",
         [](UINav &)
         {
             odometer.resetTrip();
         }});

    settingsMenu.addItem(
        MenuItem(
            "OTA Update",
            nullptr,
            &otaScreen));

    settingsMenu.addItem(
        MenuItem(
            "Vehicle Info",
            nullptr,
            &infoScreen));

    ui.setMenu(
        settingsMenu);

    ui.begin(
        I2C_FUEL,
        pico,
        dashScreen);
}

// ============================================================================
// Loop
// ============================================================================

void loop()
{
    // Pico must update first so all consumers
    // see fresh state this tick.
    pico.update();

    analogs.update();
    digitalInputs.update();

    // ======================
    // Power
    // ======================

    if (!digitalInputs.battery())
    {
        enterDeepSleep(true);
    }

    // ======================
    // RPM
    // ======================

    rpmInput.update();
    const uint16_t currentRpm = rpmInput.rpm();
    rpm.setRPM(currentRpm);

    // ======================
    // Speed / odometer
    // ======================

    const float currentSpeed =
        pico.speedKph();

    if (pico.hasData())
    {
        odometer.update(
            pico.pulseCount());
    }

    speedo.setSpeed(
        currentSpeed);

    // ======================
    // Warning lamps
    // ======================

    dash.setBrakes(
        digitalInputs.brake());

    dash.setOil(
        digitalInputs.oil());

    dash.setIndicators(
        digitalInputs.indicators());

    dash.setHighBeam(
        digitalInputs.highBeam());

    dash.setHeadlights(
        digitalInputs.lights());

    dash.setFogLights(
        digitalInputs.fog());

    dash.setBattery(
        digitalInputs.battery());

    dash.setLowFuel(
        analogs.fuelPercent() <
        LOW_FUEL_THRESHOLD);

    dash.update();

    // ======================
    // Dashboard state
    // ======================

    DashState ds;

    ds.fuelPct =
        analogs.fuelPercent();

    ds.tempPct =
        analogs.tempPercent();

    ds.odoKm =
        odometer.km();

    ds.tripKm =
        odometer.tripKm();

    ds.speedKph =
        currentSpeed;

    // ======================
    // UI
    // ======================

    dashScreen.setState(ds);

    ui.update();

    dashboard.update(ds);

    // ======================
    // RPM startup sweep
    // ======================

    static bool startupSweepStarted = false;

    if (!startupSweepStarted)
    {
        rpm.startSweep();
        startupSweepStarted = true;
    }

    rpm.update();

    // ========================================================================
    // RPM logging
    // ========================================================================

    static uint32_t lastRpmLogMs = 0;
    static uint16_t lastLoggedRPM = 0;

    const uint32_t nowMs = millis();

    const int32_t delta =
        static_cast<int32_t>(currentRpm) -
        static_cast<int32_t>(lastLoggedRPM);

    const bool suspiciousJump =
        abs(delta) >= 250;

    const bool periodicLog =
        (nowMs - lastRpmLogMs) >= 500;

    if (suspiciousJump || periodicLog)
    {
        Serial.printf(
            "[RPM] t=%lu rpm=%u delta=%ld%s\n",
            nowMs,
            currentRpm,
            delta,
            suspiciousJump
                ? "  <-- JUMP"
                : "");

        lastLoggedRPM = currentRpm;
        lastRpmLogMs = nowMs;
    }

    // ========================================================================
    // Analog calibration logging
    // ========================================================================

    static uint32_t lastAnalogLogMs = 0;

    if ((nowMs - lastAnalogLogMs) >= 2000)
    {
        int tempRaw = analogRead(PIN_TEMP_ADC);
        int fuelRaw = analogRead(PIN_FUEL_ADC);

        Serial.print("[ADC] TEMP=");
        Serial.print(tempRaw);

        Serial.print(" | FUEL=");
        Serial.println(fuelRaw);

        lastAnalogLogMs = nowMs;
    }

    // ========================================================================
    // Odometer logging
    // ========================================================================

    static uint32_t lastOdoLogMs = 0;

    if ((nowMs - lastOdoLogMs) >= 1000)
    {
        Serial.printf(
            "[ODO] pulses=%lu  odo=%.3f km  trip=%.3f km\n",
            static_cast<unsigned long>(pico.pulseCount()),
            odometer.km(),
            odometer.tripKm());

        lastOdoLogMs = nowMs;
    }
}