#include <Arduino.h>
#include "HardwareConfig.h"
#include <ButtonInput.h>
#include <SpeedInput.h>
#include <I2CSlave.h>

#define LED_PIN 8  // Built-in LED on SuperMini

ButtonInput buttonOk(PIN_BUTTON_OK, ButtonInput::BUTTON_OK, true);
ButtonInput buttonNext(PIN_BUTTON_NEXT, ButtonInput::BUTTON_NEXT, true);
SpeedInput speedInput(PIN_SPEED_SENSOR, SPEEDO_METERS_PER_PULSE);
I2CSlave i2cSlave(I2C_SLAVE_ADDRESS, I2C_SDA, I2C_SCL);

unsigned long buttonPressStart = 0;
bool longPressDetected = false;
bool blinking = false;
unsigned long blinkEndTime = 0;

void blinkLED(int duration) {
  digitalWrite(LED_PIN, LOW);  // LED on (inverted on SuperMini)
  blinking = true;
  blinkEndTime = millis() + duration;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // LED off initially
  
  buttonOk.begin();
  buttonNext.begin();
  speedInput.begin();
  i2cSlave.begin();
}

void loop() {
  buttonOk.update();
  buttonNext.update();
  speedInput.update();
  
  // Handle LED blinking
  if (blinking && millis() >= blinkEndTime) {
    digitalWrite(LED_PIN, HIGH);  // LED off
    blinking = false;
  }
  
  // Button OK - single press = short blink
  if (buttonOk.wasPressed()) {
    blinkLED(100);  // 100ms blink
  }
  
  // Button OK - long press detection
  if (buttonOk.isPressed()) {
    if (buttonPressStart == 0) {
      buttonPressStart = millis();
      longPressDetected = false;
    } else if (!longPressDetected && (millis() - buttonPressStart > 1000)) {
      longPressDetected = true;
      blinkLED(500);  // 500ms long blink
    }
  } else {
    buttonPressStart = 0;
    longPressDetected = false;
  }
  
  // Button Next - single press = medium blink
  if (buttonNext.wasPressed()) {
    blinkLED(200);  // 200ms blink
  }
  
  // Speed sensor - blink on each pulse
  static float lastSpeed = 0;
  float currentSpeed = speedInput.speedKph();
  if (currentSpeed > lastSpeed + 0.5) {  // Speed increased (pulse detected)
    blinkLED(50);  // Quick blink for pulse
  }
  lastSpeed = currentSpeed;
  
  // Update I2C data
  i2cSlave.setButtonOk(buttonOk.isPressed());
  i2cSlave.setButtonNext(buttonNext.isPressed());
  i2cSlave.setSpeedKph(speedInput.speedKph());
  i2cSlave.update();
  
  delay(10);
}
