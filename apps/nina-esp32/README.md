# NINA - ESP32

This is the ESP32 implementation of the NINA dashboard system.

## Building

Navigate to this directory first:

```bash
cd apps/nina-esp32
pio run
```

## Uploading

```bash
cd apps/nina-esp32
pio run --target upload
```

## Monitoring Serial Output

```bash
cd apps/nina-esp32
pio device monitor
```

## OTA Updates

See `OTA_README.md` for instructions on over-the-air updates.

## Configuration

Edit `include/HardwareConfig.h` to configure pin assignments and calibration values for your hardware setup.

## PlatformIO Commands

All PlatformIO commands should be run from this directory (`apps/nina-esp32/`):

- `pio run` - Build the project
- `pio run --target upload` - Build and upload to ESP32
- `pio run --target clean` - Clean build files
- `pio device monitor` - Monitor serial output
- `pio test` - Run tests (if configured)
