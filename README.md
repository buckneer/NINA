# NINA

A comprehensive, modern car dashboard system built on ESP32, featuring real-time sensor monitoring, digital displays, and LED indicators. The system is designed as a modular two-PCB solution for maximum flexibility and reliability.

---

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Hardware Components](#hardware-components)
- [Bill of Materials (BOM)](#bill-of-materials-bom)
- [PCB Design](#pcb-design)
- [Pin Assignments](#pin-assignments)
- [Software Architecture](#software-architecture)
- [Installation & Building](#installation--building)
- [Configuration](#configuration)
- [Calibration](#calibration)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Technical Specifications](#technical-specifications)
- [Future Enhancements](#future-enhancements)
- [Contributing](#contributing)
- [Safety & Warnings](#safety--warnings)
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

The ASCII diagram above illustrates the two-PCB architecture. PCB 1 handles all sensor inputs and processing, while PCB 2 manages all display outputs. Communication between the boards uses I2C buses and shift register control lines.

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

## Bill of Materials (BOM)

Complete parts list for both PCBs. All quantities are per complete system.

### PCB 1: NinaBrain (Input & Processing Board)

| Reference                        | Component              | Value/Part Number  | Quantity | Notes                       |
| -------------------------------- | ---------------------- | ------------------ | -------- | --------------------------- |
| **U1**                           | ESP32 Module           | ESP32-DEVKITC-32D  | 1        | Main microcontroller        |
| **VR1**                          | Voltage Regulator      | LM317T             | 1        | Battery Exciter regulator   |
| **U2, U4**                       | Optocoupler            | 4N35 (DIP-6)       | 2        | Digital input isolation     |
| **U5, U6**                       | Optocoupler Array      | PS2501-4 (DIP-16)  | 2        | Multi-channel isolation     |
| **U3**                           | Comparator             | LM393N (DIP-8)     | 1        | Signal conditioning         |
| **C1**                           | Ceramic Capacitor      | 0.1µF (104)        | 1        | Decoupling                  |
| **C2, C4, C5**                   | Ceramic Capacitor      | 1nF                | 3        | Filtering                   |
| **C3**                           | Electrolytic Capacitor | 0.1mF (100µF)      | 1        | Power supply filtering      |
| **D1**                           | Rectifier Diode        | 1N4007             | 1        | Power protection            |
| **D2, D4**                       | Signal Diode           | 1N4148             | 2        | General purpose             |
| **D3**                           | Schottky Diode         | 1N5819             | 1        | Power supply                |
| **D5, D6, D7**                   | Zener Diode            | 4.7V               | 3        | Voltage clamping            |
| **R1**                           | Resistor               | 10Ω                | 1        | Current limiting            |
| **R2, R3, R11, R14**             | Resistor               | 1kΩ                | 4        | Pull-up/pull-down           |
| **R4, R6**                       | Resistor               | 2.2kΩ              | 2        | Voltage divider             |
| **R5, R7, R8, R15-R20**          | Resistor               | 10kΩ               | 9        | Pull-up/pull-down           |
| **R9, R12**                      | Resistor               | 6.8kΩ              | 2        | Voltage divider (temp/fuel) |
| **R10, R13**                     | Resistor               | 2kΩ                | 2        | Voltage divider (temp/fuel) |
| **R23, R24, R25, R26, R29, R30** | Resistor               | 6.8kΩ              | 6        | Additional dividers         |
| **RV1**                          | Trimmer Potentiometer  | Bourns 3296W 100kΩ | 1        | Calibration                 |
| **J1**                           | Connector              | JST XH 1×02        | 1        | Power input                 |
| **J2**                           | Connector              | JST XH 1×02        | 1        | Pickup (RPM)                |
| **J3**                           | Connector              | JST XH 1×02        | 1        | Fuel and Temp               |
| **J4, J5, J6, J7, J10, J11**     | Connector              | JST XH 1×02        | 6        | Indicator inputs            |
| **J8, J9, J12**                  | Connector              | JST EH 1×03        | 3        | I2C/Control                 |
| **J13**                          | Connector              | JST EH 1×04        | 1        | Multi-signal                |
| **J14**                          | Connector              | JST EH 1×03        | 1        | Board power                 |
| **J15**                          | Connector              | JST XH 1×02        | 1        | 5V power                    |

### PCB 2: YugoEvoDash (Display Board)

| Reference                      | Component         | Value/Part Number        | Quantity | Notes                                      |
| ------------------------------ | ----------------- | ------------------------ | -------- | ------------------------------------------ |
| **FuelGauge1, TempGauge1**     | OLED Display      | SSD1306 0.91" 128×32     | 2        | I2C, 4-pin                                 |
| **ODO1**                       | LCD Display       | RC1602A-I2C (16×2)       | 1        | HD44780-compatible with I2C backpack       |
| **U1, U2, U3, U5, U6, U7, U8** | Shift Register    | 74HC595 (DIP-16)         | 7        | 4× RPM, 2× Speedo, 1× Dash                 |
| **U4**                         | 7-Segment Display | E1-3056-CUR-1            | 1        | Optional speedometer digit                 |
| **D1-D41**                     | LED               | 5mm LED (Various colors) | 41       | Display LEDs (29× RPM, 8× Dash, 4× others) |
| **R1-R4**                      | Resistor          | 160Ω                     | 4        | LED current limiting                       |
| **R5-R49**                     | Resistor          | Various (see schematic)  | 45       | LED current limiting, pull-ups             |
| **J1**                         | Connector         | JST EH 1×13              | 1        | Main interface to PCB 1                    |
| **J2**                         | Connector         | JST EH 1×03              | 1        | Additional interface                       |

### Additional Components (Not on PCBs)

| Component           | Quantity | Notes                       |
| ------------------- | -------- | --------------------------- |
| PCB 1 (NinaBrain)   | 1        | Manufactured board          |
| PCB 2 (YugoEvoDash) | 1        | Manufactured board          |
| USB Cable           | 1        | For programming ESP32       |
| Enclosure           | 1        | Optional, for protection    |
| Mounting Hardware   | -        | Standoffs, screws as needed |

**Enclosure:**

_Enclosure: Complete assembled system in protective enclosure showing both PCBs mounted and all displays visible. The enclosure protects the electronics from environmental factors while allowing access to displays and connectors._

<img width="738" height="379" alt="Enclosure" src="https://github.com/user-attachments/assets/11748456-ab48-481d-9950-02b444c2c2d1" />

### Component Notes

**Optocouplers:**

- 4N35: Single-channel optocoupler, 6-pin DIP
- PS2501-4: 4-channel optocoupler array, 16-pin DIP
- Both provide electrical isolation for vehicle signals

**Voltage Divider Resistors:**

- R9, R12 (6.8kΩ) and R10, R13 (2kΩ) form voltage dividers for analog inputs
- Additional 6.8kΩ resistors (R23-R26, R29, R30) may be used for other sensor inputs

**LED Specifications:**

- 29 LEDs for RPM bar graph
- 8 LEDs for warning lights
- 4 additional LEDs (possibly for status or speedometer)
- Current limiting resistors: 160Ω (R1-R4) for high-brightness LEDs

**Connectors:**

- JST XH series: 2.50mm pitch, for signal connections
- JST EH series: 2.50mm pitch, for power and multi-pin connections
- All connectors use vertical mount orientation

**Capacitor Values:**

- 104 = 0.1µF (100nF)
- 1nF = 1000pF
- 0.1mF = 100µF (likely electrolytic)

---

## PCB Design

### PCB 1: Input & Processing Board

**Schematics:**

_Complete schematic overview showing all sheets and connections for PCB 1 (NinaBrain). The root schematic provides an overview of the entire board layout and component organization._

<img width="955" height="655" alt="Root" src="https://github.com/user-attachments/assets/eae793a9-00f0-461b-9c6a-c6fd9d9cc93d" />

_Schematic Page 1: ESP32 module connections, power regulation (LM317T), and main power supply circuitry. Shows 12V input filtering, 3.3V regulation, and power distribution to the microcontroller._

<img width="827" height="567" alt="Page 1" src="https://github.com/user-attachments/assets/2ae9481e-6479-4a54-a657-58d0a12339cc" />

_Schematic Page 2: Analog input conditioning circuits including voltage dividers for temperature and fuel sensors. Shows ADC input protection and filtering components (R9-R13, R23-R30) configured for ESP32 ADC channels._

<img width="827" height="567" alt="Page 2" src="https://github.com/user-attachments/assets/3babfcc0-e25f-430b-a588-1556efe9fb1b" />

_Schematic Page 3: Digital input isolation using optocouplers (4N35 and PS2501-4). Shows signal conditioning for vehicle digital inputs including brake, oil pressure, indicators, lights, and other warning signals. Includes pull-up resistors and protection diodes._

<img width="827" height="567" alt="Page 3" src="https://github.com/user-attachments/assets/7cab9301-82bf-4bf8-bb14-0b100790a2e2" />

_Schematic Page 4: Shift register control outputs, I2C bus connections, and connector pinouts. Details the connections to PCB 2 including shift register control lines (DATA, CLK, LATCH) and I2C communication buses for displays._

<img width="827" height="567" alt="Page 4" src="https://github.com/user-attachments/assets/affe0f04-790c-4a5f-905e-c6d9ff2fd6fd" />

**PCB Layout:**

_Top view of PCB 1 showing component placement and routing. The board layout demonstrates efficient use of space with clear separation between power, analog, and digital sections. All connectors are positioned for easy access and cable management._

<img width="652" height="568" alt="PCB" src="https://github.com/user-attachments/assets/85da70bf-790d-48e3-b4c4-2afbe766f098" />

**3D Render:**

_Three-dimensional view of PCB 1 showing the physical layout and component heights. This view helps visualize the final assembly and identifies any potential clearance issues with connectors or tall components like the ESP32 module and voltage regulator heatsink._

<img width="474" height="399" alt="3D render" src="https://github.com/user-attachments/assets/4f029aff-c06f-4215-8fa3-f2ae8fc2e759" />

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

**Schematics:**

_Complete schematic overview showing all sheets and connections for PCB 2 (YugoEvoDash). The root schematic provides an overview of the display board layout including all shift registers, displays, and LED arrays._

<img width="827" height="569" alt="Root" src="https://github.com/user-attachments/assets/f0549a58-7465-4a83-8d40-28683435b201" />

_Schematic Page 1: Main interface connector (J1) receiving signals from PCB 1, power distribution, and I2C bus connections. Shows how power and control signals are routed from the input board to various display subsystems._

<img width="827" height="569" alt="Page 1" src="https://github.com/user-attachments/assets/2302db95-faec-4ba4-afee-028469adc9f4" />

_Schematic Page 2: OLED display connections (FuelGauge1 and TempGauge1) on separate I2C buses. Shows SSD1306 display modules with their I2C pull-up resistors and power connections. Each OLED is connected to a dedicated I2C bus from PCB 1._

<img width="827" height="569" alt="Page 2" src="https://github.com/user-attachments/assets/c8964301-3133-495a-8b03-95bf86765d4c" />

_Schematic Page 3: LCD display (ODO1 - RC1602A-I2C) connection and shift register control interface. Shows the HD44780-compatible LCD with I2C backpack connected to I2C Bus 0, sharing the bus with the temperature OLED display._

<img width="827" height="569" alt="Page 3" src="https://github.com/user-attachments/assets/6e2b13b3-8dfd-4dee-95fa-d33453dee982" />

_Schematic Page 4: Shift register array for dash warning lights (1× 74HC595, U8) and speedometer (2× 74HC595, U6-U7). Shows the serial shift register chain with current-limiting resistors for LED segments and warning light LEDs. Includes 7-segment display connections for speedometer digits._

<img width="827" height="569" alt="Page 4" src="https://github.com/user-attachments/assets/39592149-b4cf-4987-b8f2-1cff14e9aab3" />

_Schematic Page 5: RPM bar graph shift register chain (4× 74HC595, U1-U3, U5) driving 29 LEDs. Shows the extensive LED array with current-limiting resistors (160Ω for high-brightness LEDs). The four shift registers are daisy-chained to provide 32 output channels (29 used for RPM display)._

<img width="827" height="569" alt="Page 5" src="https://github.com/user-attachments/assets/99eef488-4210-4ea5-b377-cefd6b3e0c75" />

**PCB Layout:**

_Top view of PCB 2 showing component placement optimized for display visibility and user interface. The layout positions displays at the front edge, shift registers in organized rows, and LEDs in their functional groups (RPM bar, warning lights, speedometer). Connectors are positioned for clean cable routing to PCB 1._

<img width="1053" height="511" alt="PCB 2" src="https://github.com/user-attachments/assets/c4e59a1e-f8c4-4657-8e9d-8fdffa92bf1c" />

**3D Render:**

_Three-dimensional view of PCB 2 showing the physical layout from the user-facing side. This view demonstrates the display placement (OLEDs and LCD), LED positions, and overall panel layout. The 3D render helps visualize the final dashboard appearance and component clearances, especially important for display mounting and LED visibility._

<img width="738" height="379" alt="3D render" src="https://github.com/user-attachments/assets/c8e90ab8-bceb-4a6f-88d0-a38a207dda30" />

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

- GitHub Issues: https://buckneer/NINA/issues
- Documentation: See this README and code comments

---

---

**Last Updated:** December 27, 2025  
**Version:** 1.0.0

---

_This documentation is maintained as part of the NINA project. For questions or contributions, please open an issue on GitHub._
