# Quick Reference Guide

Fast reference for common tasks and commands.

## I2C Commands

### Basic Device Info

```bash
# Scan for devices (should show 0x42)
i2cdetect -y 1

# Read device ID (returns 0x52)
i2cget -y 1 0x42 0x00

# Read firmware version
i2cget -y 1 0x42 0x01  # Major
i2cget -y 1 0x42 0x02  # Minor
i2cget -y 1 0x42 0x03  # Patch
```

### VU Meter Control

```bash
# Enable VU meters (set control register)
i2cset -y 1 0x42 0x10 0x09

# Set left VU meter (0-255)
i2cset -y 1 0x42 0x20 128   # 50%
i2cset -y 1 0x42 0x20 255   # 100%
i2cset -y 1 0x42 0x20 0     # Off

# Set right VU meter (0-255)
i2cset -y 1 0x42 0x21 192   # 75%

# Set VU mode
i2cset -y 1 0x42 0x22 0x00  # Normal
i2cset -y 1 0x42 0x22 0x01  # Peak hold
i2cset -y 1 0x42 0x22 0xFF  # Off
```

### Backlight Control

```bash
# Enable backlight
i2cset -y 1 0x42 0x10 0x11  # Enable + backlight enable

# Set brightness (0-255)
i2cset -y 1 0x42 0x30 128   # 50%
i2cset -y 1 0x42 0x30 255   # 100%

# Set mode
i2cset -y 1 0x42 0x31 0x00  # Manual
i2cset -y 1 0x42 0x31 0x01  # Auto (based on VU)
```

### Tape Motor Control

```bash
# Enable tape motor
i2cset -y 1 0x42 0x10 0x21  # Enable + tape enable

# Set speed (0-255)
i2cset -y 1 0x42 0x40 100   # ~40%

# Set direction
i2cset -y 1 0x42 0x41 0x00  # Stop
i2cset -y 1 0x42 0x41 0x01  # Forward
i2cset -y 1 0x42 0x41 0x02  # Reverse
i2cset -y 1 0x42 0x41 0x03  # Brake
```

### Read Inputs

```bash
# Read digital inputs (bits 0-7 = inputs 1-8)
i2cget -y 1 0x42 0x50

# Read digital inputs (bits 0-3 = inputs 9-12)
i2cget -y 1 0x42 0x51

# Read encoder position (2 bytes, little-endian)
i2cget -y 1 0x42 0x60  # Low byte
i2cget -y 1 0x42 0x61  # High byte

# Read encoder button
i2cget -y 1 0x42 0x63
# 0x00 = Released
# 0x01 = Pressed
# 0x02 = Held
# 0x03 = Double-click
```

## Python Quick Examples

### Set VU Meters

```python
from rp2040_controller import RP2040Controller

with RP2040Controller() as c:
    c.enable_vu_meters(True)
    c.set_vu_meters(128, 192)  # Left 50%, Right 75%
```

### Read Inputs

```python
from rp2040_controller import RP2040Controller

with RP2040Controller() as c:
    inputs = c.get_digital_inputs()
    for i, pressed in enumerate(inputs, 1):
        if pressed:
            print(f"Input {i} pressed")
```

### Monitor Encoder

```python
from rp2040_controller import RP2040Controller
import time

with RP2040Controller() as c:
    while True:
        delta = c.get_encoder_delta()
        if delta:
            print(f"Encoder moved: {delta}")
        time.sleep(0.1)
```

## Service Management (PiCorePlayer/Tiny Core)

### Control Services

```bash
# Check status
/opt/roll-streamer/init/check-services.sh

# Start services
/opt/roll-streamer/init/start-vu-meter.sh
/opt/roll-streamer/init/start-input-handler.sh

# Stop services
/opt/roll-streamer/init/stop-services.sh

# Restart services
/opt/roll-streamer/init/stop-services.sh
/opt/roll-streamer/init/start-vu-meter.sh
/opt/roll-streamer/init/start-input-handler.sh
```

### View Logs

```bash
# Follow logs (Ctrl+C to exit)
tail -f /tmp/vu-meter-daemon.log
tail -f /tmp/input-handler.log

# View recent logs
tail -n 50 /tmp/vu-meter-daemon.log
tail -n 50 /tmp/input-handler.log

# View all logs
cat /tmp/vu-meter-daemon.log
cat /tmp/input-handler.log
```

### Persistence (Important for PiCorePlayer!)

```bash
# Save changes to persist across reboots
sudo filetool.sh -b

# Check if directory is in persistence list
grep roll-streamer /opt/.filetool.lst

# Add to persistence if not present
echo "/opt/roll-streamer" | sudo tee -a /opt/.filetool.lst
```

## Firmware Build

### Build and Upload

```bash
cd firmware/

# Build firmware
make build

# Upload via USB
make upload

# Upload via UF2 bootloader
make upload-uf2
# Follow on-screen instructions

# View serial output
make monitor
```

### Troubleshooting Build

```bash
# Clean and rebuild
make clean
make build

# List available ports
make list-ports

# Show firmware info
make info
```

## Testing Commands

### Test VU Meters

```bash
# Sweep pattern
python3 /opt/roll-streamer/scripts/vu_meter_daemon.py --test

# Manual sweep
for i in {0..255}; do
    i2cset -y 1 0x42 0x20 $i
    i2cset -y 1 0x42 0x21 $i
    sleep 0.01
done
```

### Test Inputs

```bash
# Monitor all inputs
python3 << 'EOF'
from rp2040_controller import RP2040Controller
import time
with RP2040Controller() as c:
    while True:
        inputs = c.get_digital_inputs()
        print(f"\rInputs: {['#' if i else '.' for i in inputs]}", end='', flush=True)
        time.sleep(0.1)
EOF
```

### Test Encoder

```bash
# Monitor encoder
python3 << 'EOF'
from rp2040_controller import RP2040Controller
import time
with RP2040Controller() as c:
    pos = 0
    while True:
        new_pos = c.get_encoder_position()
        if new_pos != pos:
            print(f"Position: {new_pos}")
            pos = new_pos
        time.sleep(0.05)
EOF
```

## Register Map Summary

| Register | Address | R/W | Description |
|----------|---------|-----|-------------|
| DEVICE_ID | 0x00 | R | Device ID (0x52) |
| FW_VER_MAJ | 0x01 | R | Firmware major version |
| FW_VER_MIN | 0x02 | R | Firmware minor version |
| FW_VER_PATCH | 0x03 | R | Firmware patch version |
| CONTROL | 0x10 | R/W | Control register |
| STATUS | 0x11 | R | Status register |
| ERROR | 0x12 | R | Error register |
| VU_LEFT | 0x20 | R/W | Left VU level (0-255) |
| VU_RIGHT | 0x21 | R/W | Right VU level (0-255) |
| VU_MODE | 0x22 | R/W | VU mode |
| BACKLIGHT | 0x30 | R/W | Backlight brightness (0-255) |
| BACKLIGHT_MODE | 0x31 | R/W | Backlight mode |
| TAPE_SPEED | 0x40 | R/W | Tape motor speed (0-255) |
| TAPE_DIRECTION | 0x41 | R/W | Tape motor direction |
| TAPE_MODE | 0x42 | R/W | Tape motor mode |
| INPUT_STATUS_LOW | 0x50 | R | Digital inputs 1-8 |
| INPUT_STATUS_HIGH | 0x51 | R | Digital inputs 9-12 |
| INPUT_CHANGED_LOW | 0x52 | R | Input change flags 1-8 |
| INPUT_CHANGED_HIGH | 0x53 | R | Input change flags 9-12 |
| ENCODER_POS_LOW | 0x60 | R | Encoder position low byte |
| ENCODER_POS_HIGH | 0x61 | R | Encoder position high byte |
| ENCODER_DELTA | 0x62 | R | Encoder delta (signed) |
| ENCODER_BUTTON | 0x63 | R | Encoder button state |
| CONFIG_VU_FREQ | 0x70 | R/W | VU PWM frequency divider |
| CONFIG_DEBOUNCE | 0x71 | R/W | Input debounce time (ms) |
| CONFIG_OPTIONS | 0x72 | R/W | Configuration options |
| COMMAND | 0xF0 | W | Command register |

## GPIO Pin Summary

| GPIO | Function | Type | Notes |
|------|----------|------|-------|
| GP0 | VU Left IN1 | PWM Out | DRV8833 |
| GP1 | VU Left IN2 | PWM Out | DRV8833 |
| GP2 | I2C SDA | I2C | To RPi GPIO2 |
| GP3 | I2C SCL | I2C | To RPi GPIO3 |
| GP4 | VU Right IN1 | PWM Out | DRV8833 |
| GP5 | VU Right IN2 | PWM Out | DRV8833 |
| GP6 | Backlight IN1 | PWM Out | DRV8822 |
| GP7 | Backlight IN2 | PWM Out | DRV8822 |
| GP8 | Tape Motor IN1 | PWM Out | DRV8833 |
| GP9 | Tape Motor IN2 | PWM Out | DRV8833 |
| GP10 | Encoder A | Input | Pull-up enabled |
| GP11 | Encoder B | Input | Pull-up enabled |
| GP12 | Encoder Button | Input | Pull-up enabled |
| GP13-22 | Digital Inputs 1-10 | Input | Pull-up enabled |
| GP25 | Status LED | Output | Onboard LED |
| GP26-27 | Digital Inputs 11-12 | Input | Pull-up enabled |

## Common Issues

### I2C Not Working

```bash
# Check I2C is enabled
lsmod | grep i2c

# Scan for devices
i2cdetect -y 1

# Check permissions
ls -l /dev/i2c-1

# Add user to i2c group
sudo adduser tc i2c
```

### Services Not Starting

```bash
# Check dependencies
python3 -c "import smbus2"

# Check file permissions
ls -l /opt/roll-streamer/scripts/
ls -l /opt/roll-streamer/init/

# View logs
tail -n 50 /tmp/vu-meter-daemon.log
tail -n 50 /tmp/input-handler.log

# Check if running
/opt/roll-streamer/init/check-services.sh

# Restart services
/opt/roll-streamer/init/stop-services.sh
/opt/roll-streamer/init/start-vu-meter.sh
/opt/roll-streamer/init/start-input-handler.sh
```

### VU Meters Not Responding

```bash
# Test I2C communication
i2cget -y 1 0x42 0x00

# Check if enabled
i2cget -y 1 0x42 0x10

# Enable and test
i2cset -y 1 0x42 0x10 0x09
i2cset -y 1 0x42 0x20 255
i2cset -y 1 0x42 0x21 255
```

## File Locations

| File | Location |
|------|----------|
| Python library | `/opt/roll-streamer/src/rp2040_controller.py` |
| VU meter daemon | `/opt/roll-streamer/scripts/vu_meter_daemon.py` |
| Input handler | `/opt/roll-streamer/scripts/input_handler.py` |
| Init scripts | `/opt/roll-streamer/init/*.sh` |
| Startup script | `/opt/bootlocal.sh` |
| VU meter log | `/tmp/vu-meter-daemon.log` |
| Input handler log | `/tmp/input-handler.log` |
| Documentation | `/opt/roll-streamer/docs/` |

## Additional Resources

- Full documentation: [README.md](README.md)
- **PiCorePlayer installation: [docs/PICOREPLAYER_INSTALL.md](docs/PICOREPLAYER_INSTALL.md)** ⭐
- General installation guide: [docs/INSTALLATION.md](docs/INSTALLATION.md)
- I2C register map: [docs/hardware/I2C_Register_Map.md](docs/hardware/I2C_Register_Map.md)
- GPIO allocation: [docs/hardware/GPIO_Allocation.md](docs/hardware/GPIO_Allocation.md)
- Wiring diagram: [docs/hardware/Wiring_Diagram.md](docs/hardware/Wiring_Diagram.md)
