#include <Arduino.h>
#include "HardwareConfig.h"
#include <ButtonInput.h>
#include <HallSensor.h>
#include <PicoComms.h>

ButtonInput btnNext(PIN_BTN_NEXT, "NEXT");
ButtonInput btnOk  (PIN_BTN_OK,   "OK");
HallSensor  hall   (PIN_HALL, SPEEDO_METERS_PER_PULSE, SPEEDO_SAMPLE_MS);
PicoComms   comms  (I2C_SLAVE_ADDR, I2C_FREQ_HZ);

void setup() {
    Serial.begin(SERIAL_BAUD);
    // Wait up to 3 s for USB serial to enumerate; continues regardless.
    for (uint32_t t = millis(); !Serial && millis() - t < 3000;) delay(10);

    btnNext.begin();
    btnOk.begin();
    hall.begin();
    comms.begin();
    comms.send(0, 0.0f);

    Serial.println("[NINA-RPI] Ready");
}

void loop() {
    const uint32_t now = millis();

    btnNext.update();
    btnOk.update();
    hall.update();

    uint8_t buttons = 0;
    if (btnNext.pressed()) buttons |= (1u << 0);
    if (btnOk.pressed())   buttons |= (1u << 1);

    comms.send(buttons, hall.speedKph(), btnNext.holdMs(), btnOk.holdMs());

    static uint32_t lastLog = 0;
    if (now - lastLog >= 500) {
        lastLog = now;
        Serial.print("speed: ");   Serial.print(hall.speedKph()); Serial.println(" km/h");
        Serial.print("pulses: ");  Serial.println(hall.pulseCount());
    }

    delay(1);
}
