#pragma once

#include <Screen.h>

#include "WiFiManager.h"
#include "OTAService.h"

class OTAScreen : public Screen
{
public:
	OTAScreen(WiFiManager &wifi,
			  OTAService &ota)
		: _wifi(wifi),
		  _ota(ota)
	{
	}

	void onEnter(UINav &ui) override;
	void update(UINav &ui) override;
	void render(Adafruit_GFX &display) override;
	void onOk(UINav &ui) override;

	uint16_t refreshMs() const override
	{
		return 100;
	}

private:
	enum State
	{
		STARTING,
		READY,
		ERROR
	};

	WiFiManager &_wifi;
	OTAService &_ota;

	State _state = STARTING;
};