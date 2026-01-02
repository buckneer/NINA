#include "I2CSlave.h"

I2CSlave* I2CSlave::instance = nullptr;

I2CSlave::I2CSlave(uint8_t addr, uint8_t sda, uint8_t scl)
	: address(addr), sdaPin(sda), sclPin(scl)
{
	instance = this;
	memset(&dataPacket, 0, sizeof(dataPacket));
}

void I2CSlave::begin()
{
	// Simple I2C slave initialization for ESP32-C3
	// Wire.begin((int)address, sdaPin, sclPin);
	Wire.setPins(sdaPin, sclPin);
	Wire.begin(address);
	Wire.onRequest(requestEvent);
	Wire.onReceive(receiveEvent);
	requestPending = false;
}

void I2CSlave::update()
{
	// Update checksum before sending
	dataPacket.checksum = calculateChecksum(dataPacket);
}

void I2CSlave::setButtonOk(bool pressed)
{
	dataPacket.buttonOk = pressed ? 1 : 0;
}

void I2CSlave::setButtonNext(bool pressed)
{
	dataPacket.buttonNext = pressed ? 1 : 0;
}

void I2CSlave::setSpeedKph(float speed)
{
	// Store speed as integer × 10 for precision (e.g., 12.5 km/h = 125)
	dataPacket.speedKph_x10 = (uint16_t)(speed * 10.0f);
}

void I2CSlave::onRequest()
{
	// Master is requesting data
	requestPending = true;
	
	// Send data packet
	Wire.write((uint8_t*)&dataPacket, sizeof(dataPacket));
	
	requestPending = false;
}

void I2CSlave::onReceive(int numBytes)
{
	// Master is sending data (if needed in future)
	// For now, we just acknowledge
	while (Wire.available())
	{
		Wire.read(); // Read and discard
	}
}

void I2CSlave::requestEvent()
{
	if (instance)
	{
		instance->onRequest();
	}
}

void I2CSlave::receiveEvent(int numBytes)
{
	if (instance)
	{
		instance->onReceive(numBytes);
	}
}

uint8_t I2CSlave::calculateChecksum(const DataPacket& packet)
{
	uint8_t sum = 0;
	const uint8_t* bytes = (const uint8_t*)&packet;
	
	// Sum all bytes except checksum itself
	for (size_t i = 0; i < sizeof(DataPacket) - 1; i++)
	{
		sum += bytes[i];
	}
	
	return ~sum; // Invert for checksum
}

