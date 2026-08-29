#include <Arduino.h>
#include "HardwareConfig.h"
#include <ButtonInput.h>
#include <HallSensor.h>
#include <PicoComms.h>

ButtonInput btnNext(PIN_BTN_NEXT, "NEXT");
ButtonInput btnOk(PIN_BTN_OK, "OK");

HallSensor hall(
    PIN_HALL,
    SPEEDO_METERS_PER_PULSE,
    SPEEDO_SAMPLE_MS);

PicoComms comms(
    I2C_SLAVE_ADDR,
    I2C_FREQ_HZ);

void setup()
{
    Serial.begin(SERIAL_BAUD);

    // Wait up to 3 seconds for USB serial.
    for (uint32_t t = millis();
         !Serial && millis() - t < 3000;)
    {
        delay(10);
    }

    btnNext.begin();
    btnOk.begin();

    hall.begin();
    comms.begin();

    // Initial clean state.
    comms.send(
        0,
        0.0f,
        hall.pulseCount(),
        0,
        0);

    Serial.println("[NINA-RPI] Ready");
}

void loop()
{
    btnNext.update();
    btnOk.update();
    hall.update();

    uint8_t buttons = 0;

    if (btnNext.pressed())
        buttons |= (1u << 0);

    if (btnOk.pressed())
        buttons |= (1u << 1);

    comms.send(
        buttons,
        hall.speedKph(),
        hall.pulseCount(),
        btnNext.holdMs(),
        btnOk.holdMs());

    static uint32_t lastLog = 0;
    const uint32_t now = millis();

    if (now - lastLog >= 500)
    {
        lastLog = now;

        Serial.print("speed: ");
        Serial.print(hall.speedKph(), 1);
        Serial.println(" km/h");

        Serial.print("pulses: ");
        Serial.println(hall.pulseCount());
    }

    delay(1);
}