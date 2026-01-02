# NINA - ESP32-C3

This is the ESP32-C3 implementation of the NINA dashboard system. The ESP32-C3 acts as a secondary controller that reads button inputs and speed sensor data, then sends this information to the main ESP32 board via I2C.

## Features

- **Button Input**: Reads two buttons (OK and Next) for menu control
- **Speed Sensor**: Reads hall effect sensor for vehicle speed
- **I2C Slave**: Sends button states and speed data to main ESP32 board

## Building

Navigate to this directory first:

```bash
cd apps/nina-esp32c3
pio run
```

## Uploading

```bash
cd apps/nina-esp32c3
pio run --target upload
```

## Monitoring Serial Output

```bash
cd apps/nina-esp32c3
pio device monitor
```

## Configuration

Edit `include/HardwareConfig.h` to configure pin assignments:

- **Button Pins**: `PIN_BUTTON_OK` and `PIN_BUTTON_NEXT`
- **Speed Sensor Pin**: `PIN_SPEED_SENSOR`
- **I2C Pins**: `I2C_SDA` and `I2C_SCL`
- **I2C Address**: `I2C_SLAVE_ADDRESS` (default: 0x42)
- **Speed Calibration**: `HALL_PULSES_PER_MILE` and `SPEEDO_METERS_PER_PULSE`

## Hardware Connections

### Buttons
- **Button OK (ID 1)**: Connect to `PIN_BUTTON_OK` (default: GPIO 2)
- **Button Next (ID 2)**: Connect to `PIN_BUTTON_NEXT` (default: GPIO 3)
- Buttons should be connected with pull-up resistors (internal pull-ups are used)

### Speed Sensor
- **Hall Effect Sensor**: Connect to `PIN_SPEED_SENSOR` (default: GPIO 4)
- Sensor should output pulses proportional to wheel rotation

### I2C Connection to ESP32
- **SDA**: Connect to `I2C_SDA` (default: GPIO 6) on ESP32-C3
- **SCL**: Connect to `I2C_SCL` (default: GPIO 7) on ESP32-C3
- Connect to a separate I2C bus on the main ESP32 board (not the display bus)

## I2C Communication

The ESP32-C3 acts as an I2C slave device (address 0x42) and sends data packets containing:
- Button states (OK and Next)
- Speed in km/h (×10 for precision)

See [shared/i2c-protocol/protocol.md](../../shared/i2c-protocol/protocol.md) for detailed protocol documentation.

## PlatformIO Commands

All PlatformIO commands should be run from this directory (`apps/nina-esp32c3/`):

- `pio run` - Build the project
- `pio run --target upload` - Build and upload to ESP32-C3
- `pio run --target clean` - Clean build files
- `pio device monitor` - Monitor serial output
- `pio test` - Run tests (if configured)

## ESP32-C3 Specific Notes

- **GPIO Pins**: ESP32-C3 has fewer GPIO pins than ESP32. Check the pinout for your specific board.
- **I2C**: ESP32-C3 supports I2C slave mode for communication with main board.
- **No WiFi**: ESP32-C3 doesn't have WiFi (unlike ESP32), so this version focuses on sensor reading and I2C communication.

## Troubleshooting

### Buttons Not Working
- Check button connections and wiring
- Verify pull-up resistors (internal pull-ups are enabled by default)
- Check serial monitor for button press messages

### Speed Sensor Not Reading
- Verify sensor is connected to correct GPIO pin
- Check sensor power supply
- Adjust `HALL_PULSES_PER_MILE` calibration value if speed readings are incorrect

### I2C Communication Issues
- Verify I2C connections (SDA/SCL) between ESP32-C3 and main ESP32
- Check that I2C addresses don't conflict
- Ensure proper pull-up resistors on I2C bus (typically 4.7kΩ)
- Use I2C scanner on ESP32 master to verify ESP32-C3 is detected at address 0x42
