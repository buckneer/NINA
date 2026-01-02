#pragma once

// =================================================
// WiFi Configuration
// =================================================

// WiFi credentials are loaded from .env file via build flags
// If not provided via .env, these fallback values will be used
#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"  // Set this in .env file!
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"  // Set this in .env file!
#endif

// OTA Configuration - can be overridden in .env file
#ifndef OTA_HOSTNAME
#define OTA_HOSTNAME "NINA-Dashboard"  // Device hostname on network
#endif

#ifndef OTA_PORT
#define OTA_PORT 3232  // OTA update port
#endif

#ifndef OTA_PASSWORD
#define OTA_PASSWORD "NINA_OTA_2024"  // OTA update password (change this in .env!)
#endif

// Convert macros to constexpr for use in code
constexpr const char* WIFI_SSID_STR = WIFI_SSID;
constexpr const char* WIFI_PASSWORD_STR = WIFI_PASSWORD;
constexpr const char* OTA_HOSTNAME_STR = OTA_HOSTNAME;
constexpr const char* OTA_PASSWORD_STR = OTA_PASSWORD;

// WiFi connection timeout (milliseconds)
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 30000;     // 30 seconds

// Network settings
constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 30000;  // Try to reconnect every 30 seconds if disconnected

