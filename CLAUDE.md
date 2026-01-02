# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an RP2040-based peripheral controller for a PiCorePlayer music streamer. The system connects an RPi Zero 2 running PiCorePlayer to an RP2040 microcontroller that controls VU meters, panel backlight, tape counter motor, and user inputs via I2C.

## Build Commands

### Firmware (RP2040)

```bash
cd firmware/
make build          # Build firmware
make upload         # Upload via USB (picotool)
make upload-uf2     # Build UF2 for bootloader upload
make monitor        # Serial monitor (115200 baud)
make clean          # Clean build files
```

Requires PlatformIO with earlephilhower Arduino core for RP2040.

### Python Extension Testing

```bash
# Test controller library
python3 picore-extension/src/rp2040_controller.py

# Test VU meter daemon
python3 picore-extension/scripts/vu_meter_daemon.py --test

# Test input handler
python3 picore-extension/scripts/input_handler.py --verbose
```

## Architecture

### Two-Part System

1. **RP2040 Firmware** (`firmware/`) - C++/Arduino firmware implementing I2C slave
2. **PiCorePlayer Extension** (`picore-extension/`) - Python control library and daemons

### I2C Communication

- Slave address: `0x42`
- Device ID: `0x52` (register 0x00)
- Uses register-based protocol defined in `firmware/include/i2c_registers.h`

Key registers:
- `0x10`: Control register (enable VU, backlight, tape motor)
- `0x20-0x22`: VU meters (left, right, mode)
- `0x30-0x31`: Backlight (brightness, mode)
- `0x40-0x42`: Tape motor (speed, direction, mode)
- `0x50-0x53`: Digital inputs (status and change flags)
- `0x60-0x63`: Encoder (position, delta, button state)
- `0xF0`: Command register

### GPIO Pin Layout

Defined in `firmware/include/pin_config.h`:
- GP0-1: VU Left PWM
- GP2-3: I2C1 (SDA, SCL)
- GP4-5: VU Right PWM
- GP6-7: Backlight PWM
- GP8-9: Tape motor PWM
- GP10-12: Rotary encoder (A, B, button)
- GP13-22, GP26-27: 12 digital inputs
- GP25: Status LED

### Update Rate

Firmware runs at 100Hz update loop. VU meter ballistics (300ms time constant per ANSI C16.5-1942) are implemented in the Python daemon, not firmware.

## Key Files

- `firmware/src/main.cpp` - Main firmware with I2C handlers and PWM control
- `firmware/include/i2c_registers.h` - Complete register map and protocol
- `firmware/include/pin_config.h` - All GPIO assignments
- `picore-extension/src/rp2040_controller.py` - Python control library
- `picore-extension/scripts/vu_meter_daemon.py` - VU meter daemon with ballistics
- `picore-extension/scripts/input_handler.py` - Button/encoder handler

## Hardware Notes

- VU meters, backlight, and tape motor use H-bridge motor drivers (DRV8833/DRV8822)
- All digital inputs are active-low with internal pull-ups
- 50ms debounce on digital inputs, 5ms on encoder
- PWM frequency: 1kHz, 8-bit resolution
- Encoder button detects press, hold (>1s), and double-click
