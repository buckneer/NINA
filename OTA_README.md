# OTA Updates and Monitoring Guide

## Overview

This project now supports Over-The-Air (OTA) updates and web-based monitoring, so you don't need to physically connect your ESP32 via USB cable for updates or monitoring.

## Setup

### 1. Configure WiFi Credentials

**Important:** WiFi credentials are stored in a `.env` file (gitignored) to prevent accidentally committing them to version control.

1. Copy the example file:

   ```bash
   cp .env.example .env
   ```

2. Edit `.env` and fill in your WiFi credentials:

   ```env
   WIFI_SSID=your_wifi_network_name
   WIFI_PASSWORD=your_wifi_password
   ```

3. Optionally customize OTA settings in `.env`:
   ```env
   OTA_HOSTNAME=NINA-Dashboard  # Device hostname on network
   OTA_PORT=3232                # OTA update port
   OTA_PASSWORD=NINA_OTA_2024   # OTA update password (change this!)
   ```

The `.env` file is automatically ignored by git (see `.gitignore`), so your credentials will never be committed to the repository. The `.env.example` file serves as a template and can be safely committed.

**Note:** If you don't create a `.env` file, the build will use fallback values and WiFi won't connect. Make sure to create `.env` from `.env.example`!

### 2. First Upload

Upload the firmware the first time using USB cable:

```bash
pio run -t upload
```

After this, you can use OTA for all subsequent updates!

## Using OTA Updates

### Option 1: Using PlatformIO (Recommended)

```bash
# Find your device IP address (check Serial Monitor after boot)
# Then upload via OTA:
pio run -t upload --upload-port <device-ip>

# Example:
pio run -t upload --upload-port 192.168.1.100

# Or use hostname (if mDNS works):
pio run -t upload --upload-port NINA-Dashboard.local
```

### Option 2: Using Arduino IDE

1. In Arduino IDE, go to **Tools** → **Port**
2. You should see "NINA-Dashboard at 192.168.1.XXX" in the list
3. Select it and upload normally

### Option 3: Using PlatformIO OTA Tool

You can also use the built-in OTA tool:

```bash
# Upload and specify OTA port
pio run -t upload --upload-port <device-ip> --upload-protocol espota
```

## Monitoring

### Serial Monitor

Connect via USB and use serial monitor as before:

```bash
pio device monitor
```

### Web Interface

Once connected to WiFi, open a web browser and navigate to:

```
http://<device-ip>
```

or

```
http://NINA-Dashboard.local
```

The web interface shows:

- WiFi connection status
- IP address and hostname
- Signal strength (RSSI)
- Device uptime
- Links to JSON APIs

### JSON APIs

**Status API:**

```
GET http://<device-ip>/status
```

Returns WiFi connection status, IP, RSSI, uptime, etc.

**Sensor Data API:**

```
GET http://<device-ip>/sensors
```

Returns current sensor readings (temperature, fuel, RPM, speed, odometer)

## Troubleshooting

### WiFi Not Connecting

- **Make sure you've created a `.env` file from `.env.example`**
- Check your WiFi credentials in `.env` file (not in `WiFiConfig.h`)
- Ensure your WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Check Serial Monitor for connection errors
- Verify the `.env` file is in the project root (same directory as `platformio.ini`)
- The device will continue to work without WiFi (dashboard functionality unaffected)

### OTA Upload Fails

- Ensure device is connected to WiFi (check Serial Monitor)
- Verify you're using the correct IP address
- Check that OTA password matches in your `.env` file
- Try restarting the device and uploading again
- Make sure no firewall is blocking port 3232

### Can't Access Web Interface

- Verify the device is connected to WiFi
- Check the IP address in Serial Monitor
- Ensure you're on the same network
- Try using the IP address instead of hostname

### Finding Device IP Address

After boot, check Serial Monitor output. You should see:

```
WiFi connected!
IP address: 192.168.1.XXX
```

## Security Notes

⚠️ **Important:**

- WiFi credentials are stored in `.env` (gitignored) - **never commit this file!**
- Change the default OTA password in your `.env` file before deployment
- The web interface has no authentication by default
- Consider adding authentication if exposing to the internet
- The `.env.example` file is safe to commit (it contains no real credentials)

## Features

✅ **OTA Updates** - Update firmware wirelessly  
✅ **Web Monitoring** - View device status in browser  
✅ **JSON APIs** - Access sensor data programmatically  
✅ **Auto Reconnect** - Automatically reconnects WiFi if connection is lost  
✅ **Graceful Degradation** - Device works normally even if WiFi fails
