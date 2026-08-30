#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager
{
public:
	bool startAccessPoint();
	void stop();

	bool isActive() const;
	bool hasClient() const;

	IPAddress ip() const;

private:
	static constexpr const char *AP_SSID = "NINA-DASH";
	static constexpr const char *AP_PASSWORD = "nina-dashboard";

	bool _active = false;
};