#include "SpeedInput.h"

volatile uint32_t SpeedInput::pulseCount = 0;
volatile uint32_t SpeedInput::lastPulseUs = 0;

static volatile uint32_t lastPulseUs = 0;
static volatile uint32_t lastPeriodUs = 0;
static volatile uint8_t validSequence = 0;

void IRAM_ATTR SpeedInput::isr()
{
	const uint32_t now = micros();

	if (lastPulseUs == 0)
	{
		lastPulseUs = now;
		return;
	}

	const uint32_t period = now - lastPulseUs;

	// Absolutely impossible pulse rate
	if (period < 10000)
		return;

	lastPulseUs = now;

	if (lastPeriodUs != 0)
	{
		const uint32_t minPeriod = lastPeriodUs / 2;
		const uint32_t maxPeriod = lastPeriodUs * 2;

		// Huge change = probably noise
		if (period < minPeriod || period > maxPeriod)
		{
			validSequence = 0;
			lastPeriodUs = period;
			return;
		}
	}

	lastPeriodUs = period;

	if (validSequence < 3)
	{
		validSequence++;
		return;
	}

	pulseCount++;
}

SpeedInput::SpeedInput(uint8_t p, float mpp)
	: pin(p), metersPerPulse(mpp)
{
}

void SpeedInput::begin()
{
	pinMode(pin, INPUT_PULLUP);

	attachInterrupt(
		digitalPinToInterrupt(pin),
		isr,
		FALLING);

	lastSampleMs = millis();
}

void SpeedInput::update()
{
	const uint32_t now = millis();
	const uint32_t dtMs = now - lastSampleMs;

	if (dtMs < 100)
		return;

	noInterrupts();
	const uint32_t pulses = pulseCount;
	pulseCount = 0;
	interrupts();

	const float distanceMeters = pulses * metersPerPulse;
	const float speedMps = distanceMeters / (dtMs / 1000.0f);
	const float speedKph = speedMps * 3.6f;

	speedFiltered =
		0.25f * speedKph +
		0.75f * speedFiltered;

	// prevent tiny residual values hanging around
	if (pulses == 0 && speedFiltered < 1.0f)
		speedFiltered = 0.0f;

	lastSampleMs = now;
}

float SpeedInput::speedKph() const
{
	return speedFiltered;
}

void SpeedInput::setMetersPerPulse(float mpp)
{
	if (mpp > 0.0f)
		metersPerPulse = mpp;
}

float SpeedInput::getMetersPerPulse() const
{
	return metersPerPulse;
}

void SpeedInput::resetPulseCounter()
{
	noInterrupts();

	pulseCount = 0;
	lastPulseUs = 0;

	interrupts();
}

uint32_t SpeedInput::pulsesSinceReset() const
{
	noInterrupts();
	const uint32_t p = pulseCount;
	interrupts();

	return p;
}