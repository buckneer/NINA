# NINA

A comprehensive, modern car dashboard system featuring real-time sensor monitoring, digital displays, and LED indicators. The system is designed as a modular two-PCB solution for maximum flexibility and reliability.

## Project Structure

```
NINA/
├── apps/
│   ├── nina-esp32/      # ESP32 implementation
│   └── nina-esp32c3/    # ESP32-C3 implementation
├── shared/
│   └── i2c-protocol/     # I2C communication protocol documentation
├── schematics/           # PCB schematics and design files
└── README.md             # This file
```

## Supported Platforms

- **ESP32**: Full-featured implementation with WiFi and OTA support
- **ESP32-C3**: Lightweight implementation for cost-sensitive applications

## Quick Start

### ESP32

Navigate to the ESP32 app directory and build/upload:

```bash
cd apps/nina-esp32
pio run              # Build
pio run --target upload    # Build and upload
pio device monitor   # Monitor serial output
```

See [apps/nina-esp32/README.md](apps/nina-esp32/README.md) for detailed instructions.

### ESP32-C3

Navigate to the ESP32-C3 app directory and build/upload:

```bash
cd apps/nina-esp32c3
pio run              # Build
pio run --target upload    # Build and upload
pio device monitor   # Monitor serial output
```

See [apps/nina-esp32c3/README.md](apps/nina-esp32c3/README.md) for detailed instructions.

### Building from Root (Alternative)

You can also build from the root directory by specifying the project path:

```bash
# Build ESP32
pio run --project-dir apps/nina-esp32

# Build ESP32-C3
pio run --project-dir apps/nina-esp32c3
```

## Architecture

The system is built on a **two-PCB architecture**:

### PCB 1: Input & Processing Board
- Microcontroller (ESP32 or Pico)
- Analog inputs with voltage dividers (temperature, fuel)
- Digital inputs with optocoupler isolation
- I2C interfaces for communication with display board
- Shift register control outputs (SPI-like interface)

### PCB 2: Display Board
- OLED displays (2× SSD1306 128×32)
- LCD display (HD44780-compatible, 16×2)
- Shift registers (74HC595) for LED multiplexing
- LED arrays for RPM bar, speedometer digits, warning lights
- Power regulation for displays and LEDs

## I2C Protocol

See [shared/i2c-protocol/protocol.md](shared/i2c-protocol/protocol.md) for detailed I2C communication protocol documentation.

## Hardware Components

### Microcontroller Options
- **ESP32-DevKitC**: Dual-core 240MHz, WiFi & Bluetooth
- **ESP32-C3**: Single-core RISC-V, cost-effective, no WiFi

### Power Supply
- **LM317T Voltage Regulator**: Converts 12V vehicle battery to 5V
- ESP32/Pico onboard regulators provide 3.3V from 5V supply

### Displays
- **2× SSD1306 OLED** (128×32 pixels, I2C, 0x3C)
- **1× HD44780-compatible LCD** (16×2, I2C backpack)

### Shift Registers
- **7× 74HC595** (8-bit serial-in, parallel-out)
  - 4× for RPM bar (32 outputs, 29 used)
  - 2× for speedometer (16 outputs, 3 digits × 7 segments)
  - 1× for warning lights (8 outputs)

## Features

- ✅ Real-time Engine Monitoring: Temperature and RPM tracking
- ✅ Fuel Level Monitoring: Analog fuel sender integration
- ✅ Digital Displays: OLED displays for fuel/temperature, LCD for status
- ✅ Speedometer: 3-digit LED display
- ✅ RPM Gauge: 29-LED bar graph display
- ✅ Warning Lights: 8-channel LED indicator system
- ✅ Digital Inputs: Brake, oil pressure, indicators, lights, etc.
- ✅ Modular Design: Two-PCB architecture for separation of concerns
- ✅ Multi-platform: Support for ESP32 and Raspberry Pi Pico

## Configuration

Each platform has its own configuration file:
- **ESP32**: `apps/nina-esp32/include/HardwareConfig.h`
- **ESP32-C3**: `apps/nina-esp32c3/include/HardwareConfig.h`

Edit these files to configure:
- Pin assignments
- Calibration values
- Sensor parameters
- Display settings

## Installation & Building

### Prerequisites

1. **PlatformIO IDE** (recommended) or PlatformIO CLI
   - VS Code extension or standalone
   - Download: https://platformio.org/

2. **Hardware Setup**
   - ESP32-DevKitC or ESP32-C3
   - USB cable for programming
   - Assembled PCBs (see schematics folder)

### Building

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd NINA
   ```

2. **Navigate to your platform**
   ```bash
   cd apps/nina-esp32  # or apps/nina-esp32c3
   ```

3. **Install dependencies**
   PlatformIO will automatically install dependencies listed in `platformio.ini`

4. **Build the project**
   ```bash
   pio run
   ```

5. **Upload to device**
   ```bash
   pio run --target upload
   ```

6. **Monitor serial output**
   ```bash
   pio device monitor
   ```

## Schematics

PCB schematics and design files are located in the `schematics/` folder.

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly on your target platform
5. Submit a pull request

## Safety & Warnings

⚠️ **IMPORTANT SAFETY INFORMATION**

- This system operates on 12V vehicle power. Ensure proper fusing and protection.
- Use optocouplers for all vehicle signal inputs to protect the microcontroller.
- Do not connect vehicle signals directly to GPIO pins.
- Ensure proper grounding between vehicle and PCB ground.
- Test thoroughly before installation in vehicle.
- This system is for educational/hobby use. Use at your own risk.

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

---

**Last Updated:** December 27, 2025  
**Version:** 1.0.0
