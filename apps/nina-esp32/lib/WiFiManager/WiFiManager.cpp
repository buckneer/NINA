#include "WiFiManager.h"

bool WiFiManager::startAccessPoint()
{
	if (_active)
		return true;

	WiFi.mode(WIFI_AP);

	if (!WiFi.softAP(AP_SSID, AP_PASSWORD))
	{
		WiFi.mode(WIFI_OFF);
		return false;
	}

	_active = true;

	Serial.print("[WiFi] AP started: ");
	Serial.println(AP_SSID);

	Serial.print("[WiFi] IP: ");
	Serial.println(WiFi.softAPIP());

	return true;
}

void WiFiManager::stop()
{
	if (!_active)
		return;

	WiFi.softAPdisconnect(true);
	WiFi.mode(WIFI_OFF);

	_active = false;

	Serial.println("[WiFi] stopped");
}

bool WiFiManager::isActive() const
{
	return _active;
}

bool WiFiManager::hasClient() const
{
	if (!_active)
		return false;

	return WiFi.softAPgetStationNum() > 0;
}

IPAddress WiFiManager::ip() const
{
	return WiFi.softAPIP();
}