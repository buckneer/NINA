#include "ButtonInput.h"

ButtonInput::ButtonInput(uint8_t p, ButtonId i, bool al)
	: pin(p), id(i), activeLow(al)
{
}

void ButtonInput::begin()
{
	pinMode(pin, INPUT_PULLUP); // Use internal pull-up (works for both active-low and active-high)
	lastState = activeLow ? !digitalRead(pin) : digitalRead(pin);
	currentState = lastState;
	lastDebounceTime = millis();
}

void ButtonInput::update()
{
	bool reading = digitalRead(pin);
	if (activeLow)
	{
		reading = !reading; // Invert if active-low
	}

	unsigned long now = millis();

	// Debounce logic
	if (reading != lastState)
	{
		lastDebounceTime = now;
	}

	// If debounce delay has passed, update state
	if ((now - lastDebounceTime) > DEBOUNCE_DELAY_MS)
	{
		if (reading != currentState)
		{
			currentState = reading;
			if (currentState)
			{
				// Button just pressed (rising edge)
				pressedFlag = true;
			}
		}
	}

	lastState = reading;
}

bool ButtonInput::isPressed() const
{
	return currentState;
}

bool ButtonInput::wasPressed()
{
	if (pressedFlag)
	{
		pressedFlag = false; // Clear flag after reading
		return true;
	}
	return false;
}

