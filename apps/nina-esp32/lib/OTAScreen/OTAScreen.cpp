#include "OTAScreen.h"

void OTAScreen::onEnter(UINav & /*ui*/)
{
	_state = STARTING;

	if (!_wifi.startAccessPoint())
	{
		_state = ERROR;
		return;
	}

	_ota.start();

	_state = READY;
}

void OTAScreen::update(UINav & /*ui*/)
{
	if (_state != READY)
		return;

	_ota.update();
}

void OTAScreen::onOk(UINav &ui)
{
	// Never allow exiting halfway through a firmware write.
	if (_ota.isUploading())
		return;

	_ota.stop();
	_wifi.stop();

	ui.goBack();
}

void OTAScreen::render(Adafruit_GFX &display)
{
	const uint8_t W = display.width();
	const uint8_t H = display.height();

	display.setTextColor(1);
	display.setTextSize(1);

	display.setCursor(2, 2);
	display.print("OTA Update");

	display.drawFastHLine(0, 13, W, 1);

	if (_state == STARTING)
	{
		display.setCursor(2, 22);
		display.print("Starting WiFi...");
	}
	else if (_state == ERROR)
	{
		display.setCursor(2, 22);
		display.print("WiFi ERROR");
	}
	else if (_ota.isUploading())
	{
		display.setCursor(2, 18);
		display.print("Uploading...");

		char buf[8];
		snprintf(
			buf,
			sizeof(buf),
			"%u%%",
			_ota.progress());

		display.setTextSize(2);

		const uint8_t tw = strlen(buf) * 12;

		display.setCursor(
			(W - tw) / 2,
			31);

		display.print(buf);

		display.setTextSize(1);
	}
	else
	{
		display.setCursor(2, 17);

		if (_wifi.hasClient())
			display.print("Laptop connected");
		else
			display.print("Waiting for laptop");

		display.setCursor(2, 29);
		display.print("WiFi: NINA-DASH");

		display.setCursor(2, 40);
		display.print("IP: ");
		display.print(_wifi.ip());
	}

	display.drawFastHLine(0, H - 12, W, 1);

	display.setCursor(2, H - 9);

	if (_ota.isUploading())
		display.print("Do not power off");
	else
		display.print("OK: exit");
}