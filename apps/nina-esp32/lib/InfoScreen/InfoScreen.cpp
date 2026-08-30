#include "InfoScreen.h"
#include "VehicleConfig.h"

void InfoScreen::onEnter(UINav & /*ui*/)
{
	_page = 0;
}

void InfoScreen::onNext(UINav & /*ui*/)
{
	_page++;

	if (_page >= PAGE_COUNT)
		_page = 0;
}

void InfoScreen::onOk(UINav &ui)
{
	ui.goBack();
}

void InfoScreen::render(Adafruit_GFX &display)
{
	const uint8_t W = display.width();
	const uint8_t H = display.height();

	display.setTextColor(1);
	display.setTextSize(1);

	// Header
	display.setCursor(2, 2);
	display.print("Vehicle Info");

	display.drawFastHLine(0, 13, W, 1);

	if (_page == 0)
	{
		// Software version
		display.setCursor(2, 18);
		display.print("Software: ");
		display.print(SOFTWARE_VERSION);

		// Production year
		display.setCursor(2, 29);
		display.print("Year: ");
		display.print(VEHICLE_YEAR);

		// Registration
		display.setCursor(2, 40);
		display.print("Reg: ");
		display.print(REGISTRATION_UNTIL);
	}
	else
	{
		// VIN
		display.setCursor(2, 18);
		display.print("VIN:");

		display.setCursor(2, 30);
		display.print(VEHICLE_VIN);
	}

	// Footer
	display.drawFastHLine(0, H - 12, W, 1);

	display.setCursor(2, H - 9);
	display.print("NEXT:more  OK:back");
}