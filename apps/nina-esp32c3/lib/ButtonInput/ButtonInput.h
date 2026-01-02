#pragma once
#include <Arduino.h>

class ButtonInput
{
public:
	enum ButtonId
	{
		BUTTON_OK = 1,
		BUTTON_NEXT = 2
	};

	ButtonInput(uint8_t pin, ButtonId id, bool activeLow = true);

	void begin();
	void update();

	bool isPressed() const;
	bool wasPressed(); // Returns true once per press (edge detection)
	ButtonId getId() const { return id; }

private:
	uint8_t pin;
	ButtonId id;
	bool activeLow;
	bool lastState = false;
	bool currentState = false;
	bool pressedFlag = false; // Edge detection flag

	unsigned long lastDebounceTime = 0;
	static constexpr unsigned long DEBOUNCE_DELAY_MS = 50;
};

