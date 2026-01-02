#pragma once
#include <Arduino.h>

class SpeedInput
{
public:
	SpeedInput(uint8_t pin, float metersPerPulse);

	void begin();
	void update();

	float speedKph() const;

	// -------- calibration --------
	void setMetersPerPulse(float mpp);
	float getMetersPerPulse() const;

	// optional helper for calibration-by-distance
	void resetPulseCounter();
	uint32_t pulsesSinceReset() const;

private:
	static void IRAM_ATTR isr();
	static volatile uint32_t pulseCount;

	uint8_t pin;
	float metersPerPulse;

	uint32_t lastSampleMs = 0;
	float speedFiltered = 0.0f;
};