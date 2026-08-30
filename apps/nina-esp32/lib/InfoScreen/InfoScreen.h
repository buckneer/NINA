#pragma once

#include <Screen.h>

class InfoScreen : public Screen
{
public:
	void onEnter(UINav &ui) override;
	void render(Adafruit_GFX &display) override;
	void onNext(UINav &ui) override;
	void onOk(UINav &ui) override;

	uint16_t refreshMs() const override { return 250; }

private:
	uint8_t _page = 0;

	static constexpr uint8_t PAGE_COUNT = 2;
};