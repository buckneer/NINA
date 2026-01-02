#include "SpeedInput.h"

// ======================
// ISR state
// ======================

volatile uint32_t SpeedInput::pulseCount = 0;

void IRAM_ATTR SpeedInput::isr()
{
	pulseCount++;
}

// ======================
// Implementation
// ======================

SpeedInput::SpeedInput(uint8_t p, float mpp)
	: pin(p), metersPerPulse(mpp) {}

void SpeedInput::begin()
{
	pinMode(pin, INPUT);
	attachInterrupt(digitalPinToInterrupt(pin), isr, CHANGE);
	lastSampleMs = millis();
}

void SpeedInput::update()
{
	uint32_t now = millis();
	uint32_t dtMs = now - lastSampleMs;

	if (dtMs < 100)
		return; // 10 Hz update

	noInterrupts();
	uint32_t pulses = pulseCount;
	pulseCount = 0;
	interrupts();

	float distanceMeters = pulses * metersPerPulse;
	float speedMps = distanceMeters / (dtMs / 1000.0f);
	float speedKph = speedMps * 3.6f;

	// smoothing
	speedFiltered =
		0.25f * speedKph +
		0.75f * speedFiltered;

	lastSampleMs = now;
}

float SpeedInput::speedKph() const
{
	return speedFiltered;
}

// ======================
// Calibration API
// ======================

void SpeedInput::setMetersPerPulse(float mpp)
{
	if (mpp > 0.0f)
	{
		metersPerPulse = mpp;
	}
}

float SpeedInput::getMetersPerPulse() const
{
	return metersPerPulse;
}

void SpeedInput::resetPulseCounter()
{
	noInterrupts();
	pulseCount = 0;
	interrupts();
}

uint32_t SpeedInput::pulsesSinceReset() const
{
	noInterrupts();
	uint32_t p = pulseCount;
	interrupts();
	return p;
}