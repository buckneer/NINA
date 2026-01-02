#pragma once
#include <Arduino.h>
#include <Wire.h>

// I2C Slave implementation for ESP32-C3
// Sends button states and speed data to main ESP32 board

class I2CSlave
{
public:
	struct DataPacket
	{
		uint8_t buttonOk : 1;      // Button 1 (OK) pressed
		uint8_t buttonNext : 1;    // Button 2 (Next) pressed
		uint8_t reserved : 6;      // Reserved for future use
		uint16_t speedKph_x10;     // Speed in km/h × 10 (e.g., 125 = 12.5 km/h)
		uint8_t checksum;          // Simple checksum
	};

	I2CSlave(uint8_t i2cAddress, uint8_t sdaPin, uint8_t sclPin);

	void begin();
	void update();

	// Set data to send
	void setButtonOk(bool pressed);
	void setButtonNext(bool pressed);
	void setSpeedKph(float speed);

	// Check if master is requesting data
	bool isRequestPending() const { return requestPending; }

private:
	uint8_t address;
	uint8_t sdaPin;
	uint8_t sclPin;
	DataPacket dataPacket;
	bool requestPending = false;

	void onRequest();
	void onReceive(int numBytes);

	static I2CSlave* instance;
	static void requestEvent();
	static void receiveEvent(int numBytes);

	uint8_t calculateChecksum(const DataPacket& packet);
};

