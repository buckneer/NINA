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

static void enterDeepSleep()
{
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0); // wake on LOW (IGN on)
    Serial.println("[Power] IGN off — entering deep sleep");
    Serial.flush();
    esp_deep_sleep_start();
}

TwoWire I2C_FUEL(1);

Multiplex<2> speedoMux(SPD_DATA, SRCLK, SPD_LATCH, 3, SPD_MUX_MS);
Multiplex<4> rpmMux(RPM_DATA, SRCLK, RPM_LATCH, RPMMeter::CHANNELS, RPM_MUX_MS);
Multiplex<1> dashMux(DASH_DATA, SRCLK, DASH_LATCH, 1, DASH_MUX_MS);

Speedo speedo(speedoMux);
RPMMeter rpm(rpmMux, RPM_MAX);
DashLights dash(dashMux);
Dashboard dashboard;

AnalogSensors::Pins analogPins{.temp = PIN_TEMP_ADC, .fuel = PIN_FUEL_ADC};
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
AnalogSensors analogs(analogPins, analogConfig);

DigitalInputs::Pins digitalPins{
    PIN_BRAKE,
    PIN_OIL,
    PIN_INDICATORS,
    PIN_HIGH_BEAM,
    PIN_LIGHTS,
    PIN_FOG,
    PIN_BATTERY,
};
DigitalInputs digitalInputs(digitalPins);

RPMInput rpmInput(
    PIN_RPM,
    RPM_PULSES_PER_REV);
PicoLink pico;
Odometer odometer;

// ── UI objects ───────────────────────────────────────────────────────────────

UIManager ui;
DashScreen dashScreen;
ClockWidget clockWidget;
MenuScreen settingsMenu;
ClockSetScreen clockSet;

// ─────────────────────────────────────────────────────────────────────────────

void setup()
{
    pinMode(PIN_IGN_SLEEP, INPUT); // hardware pull-up via R3 — no INPUT_PULLUP needed
    if (digitalRead(PIN_IGN_SLEEP) == HIGH)
    {
        enterDeepSleep();
    }

    speedoMux.begin();
    rpmMux.begin();
    dashMux.begin();

    speedo.begin();
    rpm.begin();
    dash.begin();

    speedo.setSpeed(0);
    rpm.setRPM(0);

    Serial.begin(115200);

    Wire.begin(TEMP_SDA, TEMP_SCL, 400000);
    I2C_FUEL.begin(FUEL_SDA, FUEL_SCL, 400000);

    I2CScanner::scan(Wire, "bus0 (Wire)");
    I2CScanner::scan(I2C_FUEL, "bus1 (I2C_FUEL)");

    pico.begin(Wire, I2C_FUEL);
    dashboard.begin(Wire, I2C_FUEL);

    analogs.begin();
    digitalInputs.begin();
    rpmInput.begin();

    dashScreen.setWidget(&clockWidget);

    settingsMenu.addItem({"Set Clock", nullptr, &clockSet});
    settingsMenu.addItem({"Trip Reset", [](UINav &)
                          { odometer.resetTrip(); }});

    ui.setMenu(settingsMenu);
    ui.begin(I2C_FUEL, pico, dashScreen);
}

void loop()
{
    // Pico must be updated first so all consumers see fresh state this tick.
    pico.update();

    analogs.update();
    digitalInputs.update();

    if (!digitalInputs.battery())
    {
        enterDeepSleep();
    }
    rpmInput.update();
    odometer.update(pico.speedKph());

    rpm.setRPM(rpmInput.rpm());
    // rpm.setRPM(7000);
    speedo.setSpeed(pico.speedKph());

    dash.setBrakes(digitalInputs.brake());
    dash.setOil(digitalInputs.oil());
    dash.setIndicators(digitalInputs.indicators());
    dash.setHighBeam(digitalInputs.highBeam());
    dash.setHeadlights(digitalInputs.lights());
    dash.setFogLights(digitalInputs.fog());
    dash.setBattery(digitalInputs.battery());
    dash.setLowFuel(analogs.fuelPercent() < LOW_FUEL_THRESHOLD);

    dash.update();

    DashState ds;
    ds.fuelPct = analogs.fuelPercent();
    ds.tempPct = analogs.tempPercent();
    ds.odoKm = odometer.km();
    ds.tripKm = odometer.tripKm();
    ds.speedKph = pico.speedKph();

    dashScreen.setState(ds);
    ui.update(); // reads button edges + renders main OLED

    dashboard.update(ds); // fuel/temp bar OLEDs
}
