#include <Arduino.h>
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

TwoWire I2C_FUEL(1);

Multiplex<2> speedoMux(SPD_DATA,  SRCLK, SPD_LATCH,  3,                  SPD_MUX_MS);
Multiplex<4> rpmMux   (RPM_DATA,  SRCLK, RPM_LATCH,  RPMMeter::CHANNELS, RPM_MUX_MS);
Multiplex<1> dashMux  (DASH_DATA, SRCLK, DASH_LATCH, 1,                  DASH_MUX_MS);

Speedo     speedo(speedoMux);
RPMMeter   rpm(rpmMux);
DashLights dash(dashMux);
Dashboard  dashboard;

AnalogSensors::Pins analogPins{ .temp = PIN_TEMP_ADC, .fuel = PIN_FUEL_ADC };
AnalogSensorsConfig analogConfig{
    .adcBits        = ADC_BITS,
    .adcRefV        = ADC_REF_V,
    .adcMax         = ADC_MAX,
    .tempVTable     = TEMP_V_TABLE,
    .tempVTableSize = TEMP_V_TABLE_SIZE,
    .tempMinC       = TEMP_MIN_C,
    .tempMaxC       = TEMP_MAX_C,
    .fuelAdcVMin    = FUEL_ADC_V_MIN,
    .fuelAdcVMax    = FUEL_ADC_V_MAX,
};
AnalogSensors analogs(analogPins, analogConfig);

DigitalInputs::Pins digitalPins{
    PIN_BRAKE, PIN_OIL, PIN_INDICATORS,
    PIN_HIGH_BEAM, PIN_LIGHTS, PIN_FOG, PIN_BATTERY,
};
DigitalInputs digitalInputs(digitalPins);

RPMInput rpmInput(PIN_RPM, RPM_SAMPLE_MS, RPM_PULSES_PER_REV);
PicoLink pico;
Odometer odometer;

void setup() {
    Serial.begin(115200);
    delay(500);

    Wire.begin(TEMP_SDA, TEMP_SCL, 400000);
    delay(50);
    I2C_FUEL.begin(FUEL_SDA, FUEL_SCL, 100000);
    delay(50);

    I2CScanner::scan(Wire,     "bus0 (Wire)");
    I2CScanner::scan(I2C_FUEL, "bus1 (I2C_FUEL)");

    pico.begin(Wire, I2C_FUEL);
    dashboard.begin(Wire, I2C_FUEL);

    speedoMux.begin(); rpmMux.begin(); dashMux.begin();
    speedo.begin();    rpm.begin();    dash.begin();
    analogs.begin();   digitalInputs.begin(); rpmInput.begin();
}

void loop() {
    analogs.update();
    digitalInputs.update();
    rpmInput.update();
    pico.update();
    odometer.update(pico.speedKph());

    if (pico.nextPressed()) Serial.println("[BTN] NEXT");
    if (pico.okPressed())   Serial.println("[BTN] OK");

    rpm.setRPM(rpmInput.rpm());
    speedo.setSpeed(pico.speedKph());

    dash.setBrakes(digitalInputs.brake());
    dash.setOil(digitalInputs.oil());
    dash.setIndicators(digitalInputs.indicators());
    dash.setHighBeam(digitalInputs.highBeam());
    dash.setHeadlights(digitalInputs.lights());
    dash.setFogLights(digitalInputs.fog());
    dash.setBattery(digitalInputs.battery());
    dash.setLowFuel(analogs.fuelPercent() < LOW_FUEL_THRESHOLD);

    DashState ds;
    ds.fuelPct = analogs.fuelPercent();
    ds.tempPct = analogs.tempPercent();
    ds.odoKm   = odometer.km();
    ds.tripKm  = odometer.tripKm();
    dashboard.update(ds);
}
