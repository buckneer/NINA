# PlatformIO extra script to set build flags from .env file with defaults
# This script runs before compilation and sets build flags based on environment variables

Import("env")
import os

# Default values (used if .env variables are not set)
defaults = {
    'WIFI_SSID': 'YOUR_WIFI_SSID',
    'WIFI_PASSWORD': 'YOUR_WIFI_PASSWORD',
    'OTA_HOSTNAME': 'NINA-Dashboard',
    'OTA_PORT': '3232',
    'OTA_PASSWORD': 'NINA_OTA_2024'
}

# Function to load .env file manually
def load_env_file(env_path):
    """Load variables from .env file into environment"""
    if not os.path.exists(env_path):
        return {}
    
    env_vars = {}
    with open(env_path, 'r') as f:
        for line in f:
            line = line.strip()
            # Skip empty lines and comments
            if not line or line.startswith('#'):
                continue
            # Parse KEY=VALUE format
            if '=' in line:
                key, value = line.split('=', 1)
                key = key.strip()
                value = value.strip()
                # Remove quotes if present
                if (value.startswith('"') and value.endswith('"')) or \
                   (value.startswith("'") and value.endswith("'")):
                    value = value[1:-1]
                env_vars[key] = value
    return env_vars

# Load .env file from project root
project_dir = env['PROJECT_DIR']
env_file_path = os.path.join(project_dir, '.env')
env_vars = load_env_file(env_file_path)

# Get values from .env file first, then from system environment, then defaults
wifi_ssid = env_vars.get('WIFI_SSID') or os.getenv('WIFI_SSID', defaults['WIFI_SSID'])
wifi_password = env_vars.get('WIFI_PASSWORD') or os.getenv('WIFI_PASSWORD', defaults['WIFI_PASSWORD'])
ota_hostname = env_vars.get('OTA_HOSTNAME') or os.getenv('OTA_HOSTNAME', defaults['OTA_HOSTNAME'])
ota_port = env_vars.get('OTA_PORT') or os.getenv('OTA_PORT', defaults['OTA_PORT'])
ota_password = env_vars.get('OTA_PASSWORD') or os.getenv('OTA_PASSWORD', defaults['OTA_PASSWORD'])

# Add build flags
env.Append(CPPDEFINES=[
    ('WIFI_SSID', '\\"%s\\"' % wifi_ssid),
    ('WIFI_PASSWORD', '\\"%s\\"' % wifi_password),
    ('OTA_HOSTNAME', '\\"%s\\"' % ota_hostname),
    ('OTA_PORT', int(ota_port)),
    ('OTA_PASSWORD', '\\"%s\\"' % ota_password)
])

# Print status (hide actual credentials)
print("WiFi Config:")
print("  WIFI_SSID: %s" % ('***SET***' if wifi_ssid != defaults['WIFI_SSID'] else 'NOT SET (using default)'))
print("  OTA_HOSTNAME: %s" % ota_hostname)
print("  OTA_PORT: %s" % ota_port)

