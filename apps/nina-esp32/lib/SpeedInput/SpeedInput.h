#pragma once
#include <Arduino.h>

class SpeedInput
{
public:
	SpeedInput(uint8_t pin, float metersPerPulse);

	void begin();
	void update();

	float speedKph() const;

	void setMetersPerPulse(float mpp);
	float getMetersPerPulse() const;

	void resetPulseCounter();
	uint32_t pulsesSinceReset() const;

private:
	static void IRAM_ATTR isr();

	static volatile uint32_t pulseCount;
	static volatile uint32_t lastPulseUs;

	static constexpr uint32_t MIN_PULSE_US = 10000;

	uint8_t pin;
	float metersPerPulse;

	uint32_t lastSampleMs = 0;
	float speedFiltered = 0.0f;
};