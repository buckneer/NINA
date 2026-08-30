#pragma once

#include <Arduino.h>

class OTAService
{
public:
	void begin();

	void start();
	void stop();
	void update();

	bool isActive() const;
	bool isUploading() const;
	bool hasError() const;

	uint8_t progress() const;

private:
	bool _initialized = false;
	bool _active = false;
	bool _uploading = false;
	bool _error = false;

	uint8_t _progress = 0;
};