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

// ============================================================================
// Power
// ============================================================================

static void enterDeepSleep()
{
    esp_sleep_enable_ext0_wakeup(
        GPIO_NUM_33,
        0); // wake on LOW (IGN on)

    Serial.println(
        "[Power] IGN off — entering deep sleep");

    Serial.flush();
    esp_deep_sleep_start();
}

// ============================================================================
// RPM display slew limiter
// ============================================================================
//
// RPMInput does the actual signal processing.
//
// This final limiter protects the visible RPM bar from physically impossible
// jumps such as:
//
//      2600 -> 4300 RPM in 5 ms
//
// A real engine cannot change RPM that quickly, so we limit how fast the
// displayed value is allowed to move.
//
// IMPORTANT:
// These are NOT hard RPM jump limits.
// They are rates measured in RPM per second.
//
// Therefore genuine fast revving is still allowed.
//

static constexpr float RPM_MAX_RISE_PER_SEC = 12000.0f;
static constexpr float RPM_MAX_FALL_PER_SEC = 16000.0f;

static float displayedRPM = 0.0f;
static uint32_t lastRpmSlewUs = 0;

static uint16_t applyRpmSlewLimit(uint16_t targetRPM)
{
    const uint32_t now = micros();

    // First measurement.
    if (lastRpmSlewUs == 0)
    {
        lastRpmSlewUs = now;
        displayedRPM = static_cast<float>(targetRPM);

        return targetRPM;
    }

    const uint32_t dtUs =
        now - lastRpmSlewUs;

    lastRpmSlewUs = now;

    // RPM input says engine stopped.
    // Go directly to zero.
    if (targetRPM == 0)
    {
        displayedRPM = 0.0f;
        return 0;
    }

    const float dtSeconds =
        static_cast<float>(dtUs) /
        1000000.0f;

    const float target =
        static_cast<float>(targetRPM);

    const float maxRise =
        RPM_MAX_RISE_PER_SEC *
        dtSeconds;

    const float maxFall =
        RPM_MAX_FALL_PER_SEC *
        dtSeconds;

    // ======================
    // Rising RPM
    // ======================

    if (target > displayedRPM)
    {
        const float difference =
            target - displayedRPM;

        if (difference > maxRise)
        {
            displayedRPM += maxRise;
        }
        else
        {
            displayedRPM = target;
        }
    }

    // ======================
    // Falling RPM
    // ======================

    else if (target < displayedRPM)
    {
        const float difference =
            displayedRPM - target;

        if (difference > maxFall)
        {
            displayedRPM -= maxFall;
        }
        else
        {
            displayedRPM = target;
        }
    }

    // Safety
    if (displayedRPM < 0.0f)
    {
        displayedRPM = 0.0f;
    }

    return static_cast<uint16_t>(
        displayedRPM + 0.5f);
}

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

    .tempRTable = nullptr,
    .tempRTableSize = 0,

    .tempMinC = TEMP_MIN_C,
    .tempMaxC = TEMP_MAX_C,

    .fuelEmptyOhms = 0.0f,
    .fuelFullOhms = 0.0f,
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
Odometer odometer;

// ============================================================================
// UI
// ============================================================================

UIManager ui;
DashScreen dashScreen;
ClockWidget clockWidget;
MenuScreen settingsMenu;
ClockSetScreen clockSet;

// ============================================================================
// Setup
// ============================================================================

void setup()
{
    // Hardware pull-up via R3.
    pinMode(
        PIN_IGN_SLEEP,
        INPUT);

    if (
        digitalRead(PIN_IGN_SLEEP) ==
        HIGH)
    {
        enterDeepSleep();
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
        enterDeepSleep();
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

    odometer.update(
        currentSpeed);

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
}