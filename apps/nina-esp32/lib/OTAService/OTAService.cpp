#include "OTAService.h"

#include <ArduinoOTA.h>

void OTAService::begin()
{
	if (_initialized)
		return;

	ArduinoOTA.setHostname("NINA-DASH");

	ArduinoOTA.onStart([this]()
					   {
        _uploading = true;
        _error = false;
        _progress = 0;

        Serial.println("[OTA] Upload started"); });

	ArduinoOTA.onProgress([this](unsigned int progress,
								 unsigned int total)
						  {
        if (total == 0)
            return;

        _progress =
            static_cast<uint8_t>((progress * 100U) / total); });

	ArduinoOTA.onEnd([this]()
					 {
        _progress = 100;

        Serial.println();
        Serial.println("[OTA] Upload complete"); });

	ArduinoOTA.onError([this](ota_error_t error)
					   {
        _uploading = false;
        _error = true;

        Serial.print("[OTA] Error: ");
        Serial.println(static_cast<int>(error)); });

	_initialized = true;
}

void OTAService::start()
{
	if (_active)
		return;

	if (!_initialized)
		begin();

	_uploading = false;
	_error = false;
	_progress = 0;

	ArduinoOTA.begin();

	_active = true;

	Serial.println("[OTA] Service started");
}

void OTAService::stop()
{
	if (!_active)
		return;

	// Do not shut OTA down halfway through an upload.
	if (_uploading)
		return;

	ArduinoOTA.end();

	_active = false;
	_progress = 0;
	_error = false;

	Serial.println("[OTA] Service stopped");
}

void OTAService::update()
{
	if (!_active)
		return;

	ArduinoOTA.handle();
}

bool OTAService::isActive() const
{
	return _active;
}

bool OTAService::isUploading() const
{
	return _uploading;
}

bool OTAService::hasError() const
{
	return _error;
}

uint8_t OTAService::progress() const
{
	return _progress;
}