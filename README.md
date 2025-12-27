# NINA - NINABrain Car Dashboard System

A comprehensive, modern car dashboard system built on ESP32, featuring real-time sensor monitoring, digital displays, and LED indicators. The system is designed as a modular two-PCB solution for maximum flexibility and reliability.

---

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Hardware Components](#hardware-components)
- [PCB Design](#pcb-design)
- [Pin Assignments](#pin-assignments)
- [Software Architecture](#software-architecture)
- [Installation & Building](#installation--building)
- [Configuration](#configuration)
- [Calibration](#calibration)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Technical Specifications](#technical-specifications)
- [License](#license)

---

## Overview

NINA (NINABrain) is a complete automotive dashboard replacement system that provides:

- **Real-time Engine Monitoring**: Temperature and RPM tracking
- **Fuel Level Monitoring**: Analog fuel sender integration
- **Digital Displays**: OLED displays for fuel/temperature, LCD for status
- **Speedometer**: 3-digit LED display
- **RPM Gauge**: 29-LED bar graph display
- **Warning Lights**: 8-channel LED indicator system
- **Digital Inputs**: Brake, oil pressure, indicators, lights, etc.
- **Modular Design**: Two-PCB architecture for separation of concerns

### Key Features

- ✅ ESP32-based processing with dual-core architecture
- ✅ Configurable, hardware-agnostic library design
- ✅ Time-multiplexed LED displays using shift registers
- ✅ Dual I2C buses for independent display control
- ✅ Analog sensor processing with calibration tables
- ✅ Interrupt-driven RPM and speed measurement
- ✅ Optocoupler-isolated digital inputs

---

## Architecture

The system is built on a **two-PCB architecture**:

### PCB 1: Input & Processing Board

- **ESP32** microcontroller
- **Analog inputs** with voltage dividers (temperature, fuel)
- **Digital inputs** with optocoupler isolation
- **I2C interfaces** for communication with display board
- **Shift register control** outputs (SPI-like interface)

### PCB 2: Display Board

- **OLED displays** (2× SSD1306 128×32)
- **LCD display** (HD44780-compatible, 16×2)
- **Shift registers** (74HC595) for LED multiplexing
- **LED arrays** for RPM bar, speedometer digits, warning lights
- **Power regulation** for displays and LEDs

**System Block Diagram:**

```
┌─────────────────────────────────────────────┐
│         PCB 1: Input & Processing          │
│  ┌──────────┐                              │
│  │  ESP32   │                              │
│  │          │                              │
│  │  GPIO    │── Analog Inputs (Temp/Fuel)  │
│  │  Pins    │── Digital Inputs (Optocouplers)│
│  │          │── Shift Reg Control (SPI)    │
│  │  I2C     │───┐                          │
│  └──────────┘   │                          │
└─────────────────┼──────────────────────────┘
                  │ I2C Bus
┌─────────────────┼──────────────────────────┐
│         PCB 2: Display Board               │
│                 │                          │
│  ┌──────────────▼──────────┐              │
│  │  OLED Displays (I2C)    │              │
│  │  - Fuel (Bus 1)         │              │
│  │  - Temperature (Bus 0)  │              │
│  └──────────────┬──────────┘              │
│                 │                          │
│  ┌──────────────▼──────────┐              │
│  │  LCD Display (I2C)      │              │
│  └──────────────────────────┘              │
│                                             │
│  ┌──────────────────────────┐              │
│  │  Shift Registers (74HC595)│             │
│  │  - RPM Bar (4× chips)    │              │
│  │  - Speedo (2× chips)     │              │
│  │  - Warning Lights (1×)   │              │
│  └──────────────────────────┘              │
└─────────────────────────────────────────────┘
```

> 📸 **TODO**: Add detailed block diagram image

---

## Hardware Components

### Microcontroller

- **ESP32-DevKitC** (or compatible)
  - Dual-core 240MHz processor
  - WiFi & Bluetooth (future use)
  - 34× GPIO pins
  - 12-bit ADC
  - Multiple I2C buses

### Displays

- **2× SSD1306 OLED** (128×32 pixels, I2C, 0x3C)
  - Fuel level display
  - Engine temperature display
- **1× HD44780-compatible LCD** (16×2, I2C backpack)
  - System status messages

### Shift Registers

- **7× 74HC595** (8-bit serial-in, parallel-out)
  - 4× for RPM bar (32 outputs, 29 used)
  - 2× for speedometer (16 outputs, 3 digits × 7 segments)
  - 1× for warning lights (8 outputs)

### Input Protection

- **Optocouplers** (active-low, for digital inputs)
  - 6N137 or similar
  - Protects ESP32 from vehicle electrical noise

### Analog Conditioning

- **Voltage Dividers** (shared design)
  - Top resistor: 6.8kΩ (R1)
  - Bottom resistor: 2.0kΩ (R2)
  - Ratio: ~0.227 (4.4× attenuation)
  - Used for temperature and fuel sensors

### Sensors

- **Engine Temperature**: OEM Yugo sensor (voltage-based)
- **Fuel Level**: Standard fuel sender (0.4V - 1.8V range)
- **RPM**: Ignition coil pulse (interrupt-driven)
- **Speed**: Hall effect sensor (optional)

---

## PCB Design

### PCB 1: Input & Processing Board

> 📸 **TODO**: Add PCB 1 schematic image
> 📸 **TODO**: Add PCB 1 board layout image
> 📸 **TODO**: Add PCB 1 3D render image

**Key Features:**

- ESP32 mounting and support circuitry
- Analog input conditioning circuits
- Optocoupler isolation for digital inputs
- I2C bus connectors for display board
- Shift register control outputs
- Power regulation (3.3V from 12V vehicle supply)

**Connectors:**

- J1: Vehicle 12V power input (fused)
- J2-J9: Sensor inputs (temperature, fuel, digital signals)
  - J2: Engine temperature sensor (analog)
  - J3: Fuel level sender (analog)
  - J4: RPM pulse (digital, interrupt)
  - J5-J10: Digital inputs (brake, oil, indicators, lights, etc.)
- J11: I2C connector to display board (SDA, SCL, GND, 3.3V)
- J12: Shift register control connector to display board
  - Data lines (RPM_DATA, DASH_DATA, SPD_DATA)
  - Shared clock (SRCLK)
  - Latch lines (RPM_LATCH, DASH_LATCH, SPD_LATCH)
  - Ground

---

### PCB 2: Display Board

> 📸 **TODO**: Add PCB 2 schematic image
> 📸 **TODO**: Add PCB 2 board layout image
> 📸 **TODO**: Add PCB 2 3D render image

**Key Features:**

- OLED display mounting (2×)
- LCD display mounting
- Shift register arrays with LED outputs
- Power distribution for displays and LEDs
- I2C bus routing

**Display Layout:**

```
┌─────────────────────────────────────┐
│  [Fuel OLED]    [Temp OLED]        │
│  (128×32)       (128×32)           │
│                                     │
│  [LCD Display - 16×2]              │
│  Status messages                    │
│                                     │
│  [Warning Lights: 8 LEDs]          │
│  ○ ○ ○ ○ ○ ○ ○ ○                   │
│                                     │
│  [RPM Bar: 29 LEDs]                │
│  ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓         │
│                                     │
│  [Speedometer: 3 digits]           │
│     123                            │
└─────────────────────────────────────┘
```

**Connectors:**

- J1: Power input from PCB 1 (3.3V, GND)
- J2: I2C bus from PCB 1 (SDA, SCL, GND)
- J3: Shift register control from PCB 1 (see PCB 1 connector J12)
- J4-J10: LED outputs (wired to shift registers)

---

## Pin Assignments

### Analog Inputs (ADC1 - WiFi Safe)

| Pin     | Function        | Notes                             |
| ------- | --------------- | --------------------------------- |
| GPIO 32 | Fuel Level ADC  | Voltage divider input             |
| GPIO 35 | Engine Temp ADC | Voltage divider input, input-only |

### Digital Inputs (Optocouplers, Active Low)

| Pin     | Function              | Notes                        |
| ------- | --------------------- | ---------------------------- |
| GPIO 12 | Headlights            | ⚠️ Boot strap pin            |
| GPIO 13 | Fog Lights            |                              |
| GPIO 14 | High Beam             |                              |
| GPIO 25 | Brake                 |                              |
| GPIO 26 | Oil Pressure          |                              |
| GPIO 27 | Indicators            |                              |
| GPIO 33 | Battery/Alternator D+ |                              |
| GPIO 34 | RPM Pulse             | Input-only, interrupt-driven |

### I2C Buses

| Bus       | SDA     | SCL     | Devices        | Speed  |
| --------- | ------- | ------- | -------------- | ------ |
| I2C Bus 0 | GPIO 21 | GPIO 22 | Temp OLED, LCD | 400kHz |
| I2C Bus 1 | GPIO 18 | GPIO 19 | Fuel OLED      | 400kHz |

### Shift Register Control (Shared Clock)

| Pin     | Function      | Connected To                |
| ------- | ------------- | --------------------------- |
| GPIO 23 | SRCLK (Clock) | All shift registers         |
| GPIO 2  | RPM_DATA      | RPM shift registers         |
| GPIO 15 | RPM_LATCH     | RPM shift registers         |
| GPIO 5  | DASH_DATA     | Dash lights shift register  |
| GPIO 17 | DASH_LATCH    | Dash lights shift register  |
| GPIO 16 | SPD_DATA      | Speedometer shift registers |
| GPIO 4  | SPD_LATCH     | Speedometer shift registers |

### Optional/Future

| Pin     | Function    | Notes                   |
| ------- | ----------- | ----------------------- |
| GPIO 39 | Hall Sensor | Speed input, input-only |

**⚠️ Boot Strap Pins Warning:**

- GPIO 12 must be LOW at boot (use pull-down if needed)
- GPIO 2 must be HIGH at boot (use pull-up if needed)
- GPIO 15 must be HIGH at boot (use pull-up if needed)

---

## Software Architecture

### Project Structure

```
NINA/
├── src/
│   └── main.cpp              # Application entry point
├── include/
│   └── HardwareConfig.h      # Hardware pin & calibration config
├── lib/
│   ├── AnalogSensors/        # Temperature & fuel sensors
│   ├── DigitalInputs/        # Optocoupler inputs
│   ├── RPMInput/             # RPM pulse measurement
│   ├── SpeedInput/           # Hall sensor speed
│   ├── RPM/                  # RPM bar display
│   ├── Speedo/               # Speedometer display
│   ├── DashLights/           # Warning lights
│   ├── Displays/             # OLED & LCD management
│   └── Multiplex/            # Shift register multiplexer
└── platformio.ini            # Build configuration
```

### Library Design Philosophy

All libraries are **configuration-agnostic** - they accept configuration parameters via constructors rather than including hardware configuration files. This makes the libraries reusable across different hardware configurations.

**Example:**

```cpp
// Configuration is passed from main.cpp
AnalogSensorsConfig config = {
    .adcBits = ADC_BITS,
    .adcRefV = ADC_REF_V,
    // ... other config values
};
AnalogSensors sensors(pins, config);
```

### Key Modules

#### AnalogSensors

- Reads engine temperature and fuel level
- Uses lookup table for temperature conversion
- Linear interpolation for smooth readings
- Low-pass filtering for noise reduction

#### DigitalInputs

- Reads optocoupler-isolated signals
- Active-low logic (optocoupler pulls down when active)
- Debounced inputs

#### RPMInput

- Interrupt-driven pulse counting
- Configurable pulses-per-revolution
- Sliding window averaging

#### SpeedInput

- Hall effect sensor pulse counting
- Configurable calibration (meters per pulse)
- Speed filtering for smooth display

#### Multiplex System

- Time-division multiplexing for LED displays
- Uses ESP32 Ticker for precise timing
- Supports multiple shift register chains
- Refresh rates:
  - RPM: 1ms (1000 Hz)
  - Speedometer: 1ms (1000 Hz)
  - Dash Lights: 2ms (500 Hz)

---

## Installation & Building

### Prerequisites

1. **PlatformIO IDE** (recommended) or PlatformIO CLI

   - VS Code extension or standalone
   - Download: https://platformio.org/

2. **Hardware Setup**
   - ESP32-DevKitC or compatible board
   - USB cable for programming
   - Assembled PCBs (see PCB Design section)

### Building

1. **Clone the repository**

   ```bash
   git clone <repository-url>
   cd NINA
   ```

2. **Install dependencies**
   PlatformIO will automatically install dependencies listed in `platformio.ini`:

   - Adafruit GFX Library
   - Adafruit SSD1306
   - ShiftRegister74HC595
   - hd44780

3. **Build the project**

   ```bash
   pio run
   ```

   Or use the PlatformIO IDE build button.

4. **Upload to ESP32**

   ```bash
   pio run --target upload
   ```

   Or use the PlatformIO IDE upload button.

5. **Monitor serial output**
   ```bash
   pio device monitor
   ```
   Serial output at 115200 baud.

### Configuration

All hardware configuration is in `include/HardwareConfig.h`. Edit this file to match your hardware setup:

- Pin assignments
- Calibration values
- Sensor parameters
- Display settings

See [Configuration](#configuration) section for details.

---

## Configuration

### Hardware Configuration File

Edit `include/HardwareConfig.h` to configure:

#### Pin Assignments

```cpp
constexpr uint8_t PIN_TEMP_ADC = 35;      // Temperature sensor pin
constexpr uint8_t PIN_FUEL_ADC = 32;      // Fuel sender pin
constexpr uint8_t PIN_RPM = 34;           // RPM pulse pin
// ... etc
```

#### ADC Settings

```cpp
constexpr uint8_t ADC_BITS = 12;          // ADC resolution
constexpr float ADC_REF_V = 3.3f;         // Reference voltage
```

#### Voltage Divider

```cpp
constexpr float ADC_DIV_R1 = 6800.0f;     // Top resistor (ohms)
constexpr float ADC_DIV_R2 = 2000.0f;     // Bottom resistor (ohms)
```

#### Temperature Calibration

```cpp
constexpr TempVPoint TEMP_V_TABLE[] = {
    {0.35f, 20},   // 0.35V = 20°C
    {0.55f, 40},   // 0.55V = 40°C
    // ... etc
};
```

#### Fuel Calibration

```cpp
constexpr float FUEL_ADC_V_MIN = 0.40f;   // Empty tank voltage
constexpr float FUEL_ADC_V_MAX = 1.80f;   // Full tank voltage
```

#### RPM Configuration

```cpp
constexpr uint8_t RPM_PULSES_PER_REV = 2;  // Pulses per engine revolution
constexpr uint16_t RPM_SAMPLE_MS = 200;    // Measurement window
```

#### Speed Calibration

```cpp
constexpr uint32_t HALL_PULSES_PER_MILE = 3200;  // Sensor pulses per mile
```

---

## Calibration

### Temperature Sensor Calibration

The temperature sensor uses a lookup table for conversion. To calibrate:

1. Measure voltage at known temperatures
2. Update `TEMP_V_TABLE` in `HardwareConfig.h`
3. Ensure table is ordered from cold to hot
4. Rebuild and upload

**Example:**

```cpp
constexpr TempVPoint TEMP_V_TABLE[] = {
    {0.35f, 20},   // Measured: 0.35V at 20°C
    {0.55f, 40},   // Measured: 0.55V at 40°C
    {0.80f, 60},   // Measured: 0.80V at 60°C
    // Add more points for accuracy
};
```

### Fuel Level Calibration

1. **Empty Tank:**

   - Drain fuel tank completely
   - Measure ADC voltage (should be ~0.40V)
   - Update `FUEL_ADC_V_MIN`

2. **Full Tank:**
   - Fill tank completely
   - Measure ADC voltage (should be ~1.80V)
   - Update `FUEL_ADC_V_MAX`

**Formula:**

```cpp
Fuel% = (Voltage - V_MIN) / (V_MAX - V_MIN) × 100
```

### RPM Calibration

Adjust `RPM_PULSES_PER_REV` based on your ignition system:

- **2 pulses/rev**: Standard 4-cylinder (most common)
- **1 pulse/rev**: Single-coil systems
- **4 pulses/rev**: Waste-spark systems

Use a timing light or scan tool to verify accuracy.

### Speedometer Calibration

1. Drive exactly 1 mile (or known distance)
2. Count sensor pulses using serial monitor
3. Calculate: `PULSES_PER_MILE = total_pulses / miles`
4. Update `HALL_PULSES_PER_MILE` in `HardwareConfig.h`

**Alternative method:**

- Use GPS for reference speed
- Adjust `SPEEDO_METERS_PER_PULSE` until readings match

---

## Usage

### Power-On Sequence

1. Apply 12V power to PCB 1
2. ESP32 boots and initializes:
   - I2C buses configured
   - Displays initialized
   - Sensors calibrated
   - Multiplexers started
3. LCD shows: "DASH READY" / "SENSORS ONLINE"
4. System enters operational loop

### Normal Operation

The system continuously:

- Reads analog sensors (temperature, fuel)
- Monitors digital inputs (brakes, lights, etc.)
- Calculates RPM from pulse count
- Updates all displays
- Controls warning lights

**Update Rates:**

- Sensors: Continuous (filtered)
- RPM: ~5 Hz (200ms sample window)
- Speed: ~10 Hz (100ms sample window)
- Displays: Real-time

### Serial Monitor

Connect via USB to view debug information:

```
Dashboard booting...
Setup complete
```

Error messages will appear if initialization fails (e.g., "Fuel OLED init failed").

---

## Troubleshooting

### Display Issues

**OLED Displays Not Working:**

- Check I2C connections (SDA/SCL)
- Verify I2C address (0x3C)
- Check power supply (3.3V)
- Use I2C scanner to detect devices

**LCD Not Displaying:**

- Verify I2C backpack connection
- Check backlight power
- Review serial monitor for error codes

### Sensor Issues

**Temperature Reading Incorrect:**

- Verify voltage divider values
- Check sensor wiring
- Calibrate lookup table (see Calibration section)
- Measure actual voltage with multimeter

**Fuel Level Wrong:**

- Calibrate empty/full voltages
- Check fuel sender resistance range
- Verify voltage divider circuit
- Check for wiring faults

**RPM Not Working:**

- Verify interrupt pin (GPIO 34)
- Check pulses-per-revolution setting
- Use oscilloscope to verify pulse signal
- Check for noise/interference

### Shift Register Issues

**LEDs Not Lighting:**

- Verify shift register connections (DATA, CLK, LATCH)
- Check shared clock line (SRCLK)
- Test with simple shift register test code
- Verify power supply to shift registers

**LEDs Flickering:**

- Increase refresh rate (decrease `*_MUX_MS`)
- Check for loose connections
- Verify timing calculations

### General Issues

**ESP32 Not Booting:**

- Check boot strap pins (GPIO 12, 2, 15)
- Verify power supply (3.3V stable)
- Check for short circuits
- Review serial output for boot messages

**System Resets:**

- Check power supply stability
- Review watchdog timeouts
- Check for memory issues
- Monitor serial output for error messages

**Intermittent Operation:**

- Check all connections
- Verify ground connections
- Check for electromagnetic interference
- Review power supply quality

---

## Technical Specifications

### Electrical Specifications

| Parameter            | Value               | Notes                |
| -------------------- | ------------------- | -------------------- |
| Operating Voltage    | 12V DC              | Vehicle battery      |
| Regulated Voltage    | 3.3V                | ESP32 supply         |
| Current Consumption  | ~500mA              | Typical operation    |
| ADC Resolution       | 12-bit              | 0-4095 counts        |
| ADC Reference        | 3.3V                | Internal             |
| I2C Speed            | 400kHz              | Fast mode            |
| Digital Input Logic  | Active Low          | Optocoupler isolated |
| Shift Register Clock | Software controlled | Ticker-based         |

### Display Specifications

| Display   | Type    | Resolution | Interface | Address      |
| --------- | ------- | ---------- | --------- | ------------ |
| Fuel OLED | SSD1306 | 128×32     | I2C Bus 1 | 0x3C         |
| Temp OLED | SSD1306 | 128×32     | I2C Bus 0 | 0x3C         |
| LCD       | HD44780 | 16×2       | I2C Bus 0 | Configurable |

### Sensor Specifications

| Sensor      | Type             | Range      | Notes           |
| ----------- | ---------------- | ---------- | --------------- |
| Temperature | Voltage-based    | 0-120°C    | OEM Yugo sensor |
| Fuel Level  | Resistance-based | 0-100%     | Standard sender |
| RPM         | Pulse            | 0-8000 RPM | Configurable    |
| Speed       | Hall effect      | 0-200 km/h | Optional        |

### Performance Specifications

| Metric            | Value       | Notes               |
| ----------------- | ----------- | ------------------- |
| RPM Update Rate   | ~5 Hz       | 200ms sample window |
| Speed Update Rate | ~10 Hz      | 100ms sample window |
| Display Refresh   | 500-1000 Hz | Per display type    |
| Sensor Reading    | Continuous  | Filtered            |
| Loop Time         | <10ms       | Typical             |

---

## Future Enhancements

Potential improvements and additions:

- [ ] WiFi connectivity for remote monitoring
- [ ] Data logging to SD card
- [ ] Bluetooth connectivity
- [ ] OTA (Over-The-Air) updates
- [ ] CAN bus integration
- [ ] Additional sensor inputs
- [ ] Customizable display layouts
- [ ] Alarm/alert system
- [ ] Trip computer functions
- [ ] Backlight dimming

---

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

---

## Safety & Warnings

⚠️ **IMPORTANT SAFETY INFORMATION**

- This system operates on 12V vehicle power. Ensure proper fusing and protection.
- Use optocouplers for all vehicle signal inputs to protect the ESP32.
- Do not connect vehicle signals directly to ESP32 GPIO pins.
- Ensure proper grounding between vehicle and PCB ground.
- Test thoroughly before installation in vehicle.
- This system is for educational/hobby use. Use at your own risk.
- Not responsible for any damage or accidents resulting from use of this system.

**Boot Strap Pin Requirements:**

- GPIO 12, 2, 15 have special boot requirements
- Verify pull-up/pull-down resistors are correctly installed
- See Pin Assignments section for details

---

## License

[Specify your license here]

---

## Acknowledgments

- ESP32 Arduino Core Team
- Adafruit Industries (OLED & GFX libraries)
- ShiftRegister74HC595 library contributors
- hd44780 library contributors

---

## Contact & Support

For questions, issues, or contributions:

- GitHub Issues: [repository-url]/issues
- Documentation: See this README and code comments

---

**Last Updated:** [Date]
**Version:** 1.0.0
