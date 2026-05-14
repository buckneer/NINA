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
    delay(200);

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
        printf("[TELEM] speed=%.1f km/h  btn=0x%02X\n", hall.speedKph(), buttons);
    }

    delay(1);
}
