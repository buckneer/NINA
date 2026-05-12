# NINA RP2040 (Raspberry Pi Pico) App

Basic PlatformIO project for an **RP2040 Pico-class** board.

## What it does

- Blinks a **GPIO pin** (`BLINK_PIN`)

## Build / Upload

From repo root:

```bash
pio run --project-dir apps/rpi
pio run --project-dir apps/rpi -t upload
pio device monitor --project-dir apps/rpi
```

Or from the project folder:

```bash
cd apps/rpi
pio run
pio run -t upload
pio device monitor
```

## Pick the blink pin

Set `BLINK_PIN` in `apps/rpi/platformio.ini`.

Notes:
- On a **real Raspberry Pi Pico**, the onboard LED is on **GPIO25**, so `BLINK_PIN=25` will blink it.
- If your board’s only “LED” is a **NeoPixel/WS2812**, you **cannot** blink it with `digitalWrite()`; you’ll need a NeoPixel driver (different setup), or just attach a cheap external LED+resistor to any GPIO for this baseline test.
