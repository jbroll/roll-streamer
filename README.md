# PiCorePlayer Music Streamer - RP2040 Peripheral Controller

A complete embedded control system for a PiCorePlayer-based music streamer with VU meters, backlight control, tape counter motor, and user inputs.

## System Overview

This project provides a sophisticated peripheral control system for a music streamer based on:
- **RPi Zero 2** running PiCorePlayer
- **ES9038Q2M** DAC for high-quality audio output
- **RP2040** microcontroller for peripheral control
- **VU meters** (2x) with authentic ballistics
- **Panel backlight** control
- **Tape counter motor** for visual effect
- **Digital inputs** (12x) for user control
- **Rotary encoder** for volume/navigation

## Features

### Hardware Features
- I2C slave interface for RPi communication
- Dual VU meter control with proper 300ms ballistics
- PWM backlight control with multiple modes
- Bi-directional tape motor control
- 12 debounced digital inputs
- Rotary encoder with button (press, hold, double-click detection)
- Onboard status LED

### Software Features
- Python library for easy control
- VU meter daemon with authentic ballistics
- Input handler for buttons and encoder
- PiCorePlayer/Tiny Core init scripts for automatic startup
- Comprehensive I2C register map
- Multiple operating modes

## Repository Structure

```
roll-streamer/
├── firmware/                    # RP2040 firmware (PlatformIO)
│   ├── platformio.ini          # PlatformIO configuration
│   ├── include/                # Header files
│   │   ├── pin_config.h       # GPIO pin definitions
│   │   └── i2c_registers.h    # I2C register map
│   └── src/
│       └── main.cpp            # Main firmware
├── picore-extension/           # PiCorePlayer extension
│   ├── src/
│   │   └── rp2040_controller.py  # Python control library
│   ├── scripts/
│   │   ├── vu_meter_daemon.py    # VU meter daemon
│   │   └── input_handler.py      # Input handler
│   └── init/                      # Init scripts for PiCorePlayer
│       ├── start-vu-meter.sh
│       ├── start-input-handler.sh
│       ├── stop-services.sh
│       └── check-services.sh
├── docs/                       # Documentation
│   ├── hardware/
│   │   ├── GPIO_Allocation.md     # Pin allocation
│   │   ├── I2C_Register_Map.md    # Register documentation
│   │   └── Wiring_Diagram.md      # Connection guide
│   └── software/
└── VUMeters.md                 # VU meter theory and design

```

## Quick Start

### 1. Hardware Setup

1. **Wire the RP2040 to RPi Zero 2:**
   - GP2 (SDA) → RPi GPIO2 (Pin 3)
   - GP3 (SCL) → RPi GPIO3 (Pin 5)
   - GND → RPi GND
   - Add 4.7kΩ pull-ups on SDA and SCL

2. **Connect motor drivers:**
   - DRV8833 #1 → VU meters (left and right)
   - DRV8822 → Backlight
   - DRV8833 #2 → Tape counter motor

3. **Connect inputs:**
   - 12 digital inputs to GP13-22, GP26-27
   - Rotary encoder to GP10-12
   - All inputs connect to GND when active

See [Wiring Diagram](docs/hardware/Wiring_Diagram.md) for complete connection details.

### 2. Firmware Installation

#### Using PlatformIO (Recommended)

```bash
# Install PlatformIO
pip install platformio

# Navigate to firmware directory
cd firmware/

# Build firmware
pio run

# Upload to RP2040 (connect via USB)
pio run --target upload

# Or build UF2 file and copy to RP2040 in bootloader mode
pio run --target upload --upload-port /media/RPI-RP2
```

#### Manual Build

```bash
cd firmware/
# Put RP2040 in bootloader mode (hold BOOTSEL, press RESET)
# Drag and drop the built .uf2 file to the RPI-RP2 drive
```

### 3. PiCorePlayer Extension Installation

**Note:** PiCorePlayer uses Tiny Core Linux, not systemd. See [PiCorePlayer Installation Guide](docs/PICOREPLAYER_INSTALL.md) for complete instructions.

#### Quick Install (on RPi Zero 2)

```bash
# Install required packages
tce-load -wi python3.6.tcz
tce-load -wi i2c-tools.tcz
tce-load -wi python3.6-pip.tcz
sudo pip3 install smbus2

# Copy extension files
sudo mkdir -p /opt/roll-streamer
sudo cp -r picore-extension/src /opt/roll-streamer/
sudo cp -r picore-extension/scripts /opt/roll-streamer/
sudo cp -r picore-extension/init /opt/roll-streamer/
sudo chmod +x /opt/roll-streamer/scripts/*.py
sudo chmod +x /opt/roll-streamer/init/*.sh

# Add to bootlocal.sh for automatic startup
sudo vi /opt/bootlocal.sh
# Add these lines at the end:
#   /opt/roll-streamer/init/start-vu-meter.sh &
#   /opt/roll-streamer/init/start-input-handler.sh &

# Save changes (PiCorePlayer runs from RAM!)
echo "/opt/roll-streamer" | sudo tee -a /opt/.filetool.lst
sudo filetool.sh -b
```

#### Verify Installation

```bash
# Check I2C device is detected
i2cdetect -y 1
# Should show device at address 0x42

# Start services manually
/opt/roll-streamer/init/start-vu-meter.sh
/opt/roll-streamer/init/start-input-handler.sh

# Check service status
/opt/roll-streamer/init/check-services.sh

# View logs
tail -f /tmp/vu-meter-daemon.log
tail -f /tmp/input-handler.log
```

## Usage

### Python Library

```python
from rp2040_controller import RP2040Controller

# Connect to controller
with RP2040Controller() as controller:
    # Get firmware version
    version = controller.get_firmware_version()
    print(f"Firmware: {version[0]}.{version[1]}.{version[2]}")

    # Set VU meters to 50%
    controller.enable_vu_meters(True)
    controller.set_vu_meters(128, 128)

    # Set backlight to 75%
    controller.enable_backlight(True)
    controller.set_backlight(192)

    # Read digital inputs
    inputs = controller.get_digital_inputs()
    for i, pressed in enumerate(inputs, 1):
        if pressed:
            print(f"Input {i} is pressed")

    # Read encoder
    position = controller.get_encoder_position()
    print(f"Encoder position: {position}")
```

### Command-Line Tools

```bash
# Test VU meters
python3 /opt/roll-streamer/scripts/vu_meter_daemon.py --test

# Run VU meter daemon
python3 /opt/roll-streamer/scripts/vu_meter_daemon.py --pipe /tmp/vu_meter_data

# Run input handler
python3 /opt/roll-streamer/scripts/input_handler.py --poll-rate 20
```

### I2C Direct Access

```bash
# Using i2c-tools

# Read device ID (should return 0x52)
i2cget -y 1 0x42 0x00

# Set left VU meter to 50% (value 128)
i2cset -y 1 0x42 0x20 128

# Set right VU meter to 75% (value 192)
i2cset -y 1 0x42 0x21 192

# Enable VU meters (set bit 3 in control register)
i2cset -y 1 0x42 0x10 0x09

# Read digital input status
i2cget -y 1 0x42 0x50
```

## Configuration

### Default Input Mappings

| Input | Function |
|-------|----------|
| 1 | Play/Pause |
| 2 | Stop |
| 3 | Next Track |
| 4 | Previous Track |
| 5 | Volume Up |
| 6 | Volume Down |
| 7 | Mute Toggle |
| 8-12 | User-defined |

### Rotary Encoder

- **Rotate**: Volume control (default) or track selection
- **Press**: Play/Pause
- **Hold (>1s)**: Toggle encoder mode
- **Double-click**: Mute toggle

### VU Meter Calibration

Edit `/opt/roll-streamer/scripts/vu_meter_daemon.py`:

```python
daemon = VUMeterDaemon(
    pipe_path="/tmp/vu_meter_data",
    update_rate=50,         # Update rate in Hz
    min_db=-20.0,          # Minimum dB (meter zero)
    max_db=3.0             # Maximum dB (meter full scale)
)
```

## Documentation

- [GPIO Pin Allocation](docs/hardware/GPIO_Allocation.md) - Complete pin mapping
- [I2C Register Map](docs/hardware/I2C_Register_Map.md) - Register documentation
- [Wiring Diagram](docs/hardware/Wiring_Diagram.md) - Connection guide
- [VU Meters](VUMeters.md) - VU meter theory and implementation

## Hardware Components

### Required Components

- **Microcontroller**: Raspberry Pi Pico (RP2040)
- **Main Computer**: Raspberry Pi Zero 2 W
- **DAC**: ES9038Q2M I2S DAC
- **Motor Drivers**:
  - 2x DRV8833 Dual H-Bridge (for VU meters and tape motor)
  - 1x DRV8822 Single H-Bridge (for backlight)
- **VU Meters**: 2x analog VU meters (1-5mA, 1-3V)
- **Rotary Encoder**: EC11 or similar (with button)
- **Switches/Buttons**: 12x momentary push buttons
- **Power Supply**: 5V 2.5A minimum
- **Miscellaneous**: Resistors, capacitors, connectors

### Recommended Parts

- **VU Meters**: Vintage-style panel meters or modern equivalents
- **Encoder**: Bourns PEC11R or Alps EC11
- **Buttons**: Cherry MX switches or tactile buttons
- **Case**: Custom 3D-printed or aluminum chassis

## Troubleshooting

### I2C Communication Issues

```bash
# Check I2C is enabled
ls /dev/i2c-*

# Scan for devices
i2cdetect -y 1

# If device not found:
# 1. Check wiring (SDA, SCL, GND)
# 2. Verify pull-up resistors (4.7kΩ)
# 3. Check RP2040 is powered and firmware is running
# 4. Verify I2C address (should be 0x42)
```

### VU Meters Not Moving

```bash
# Check if daemon is running
systemctl status vu-meter.service

# Test VU meters directly
python3 /opt/roll-streamer/scripts/vu_meter_daemon.py --test

# Check I2C communication
i2cget -y 1 0x42 0x11  # Read status register

# Manually set VU meters
i2cset -y 1 0x42 0x20 255  # Left VU to max
i2cset -y 1 0x42 0x21 255  # Right VU to max
```

### Digital Inputs Not Working

```bash
# Check input handler is running
systemctl status input-handler.service

# Test inputs directly
python3 -c "
from rp2040_controller import RP2040Controller
with RP2040Controller() as c:
    while True:
        inputs = c.get_digital_inputs()
        print(inputs)
        time.sleep(0.1)
"
```

### Encoder Not Responding

- Check wiring (A, B, SW pins)
- Verify pull-ups are enabled (internal or external)
- Check for proper quadrature signals with oscilloscope
- Ensure encoder is not generating too many pulses per second

## Development

### Building Firmware

```bash
cd firmware/
pio run              # Build
pio run -t clean     # Clean
pio run -t upload    # Upload
pio device monitor   # Serial monitor
```

### Testing

```bash
# Test RP2040 controller
cd picore-extension/src/
python3 rp2040_controller.py

# Test VU meter daemon
cd picore-extension/scripts/
python3 vu_meter_daemon.py --test

# Test input handler
python3 input_handler.py --verbose
```

### Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is provided as-is for educational and personal use.

## Credits

- VU meter ballistics based on ANSI C16.5-1942 standard
- Inspired by classic analog audio equipment
- Built for the PiCorePlayer community

## Support

For issues, questions, or contributions:
- Open an issue on GitHub
- Check documentation in `docs/` directory
- Review examples in `picore-extension/src/`

## Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 2025-11-05 | Initial release |

---

**Happy listening! 🎵**
