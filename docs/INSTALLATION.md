# Installation Guide

Complete step-by-step installation guide for the PiCorePlayer Music Streamer with RP2040 peripheral controller.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Hardware Assembly](#hardware-assembly)
3. [Firmware Installation](#firmware-installation)
4. [PiCorePlayer Setup](#picoreplayer-setup)
5. [Software Installation](#software-installation)
6. [Configuration](#configuration)
7. [Testing](#testing)
8. [Troubleshooting](#troubleshooting)

## Prerequisites

### Hardware Required

- [ ] Raspberry Pi Zero 2 W
- [ ] Raspberry Pi Pico (RP2040)
- [ ] ES9038Q2M I2S DAC board
- [ ] 2x DRV8833 motor driver boards
- [ ] 1x DRV8822 motor driver board
- [ ] 2x VU meters (analog, 1-5mA)
- [ ] 1x Rotary encoder (EC11 style)
- [ ] 12x Push buttons or switches
- [ ] 2x 4.7kΩ resistors (I2C pull-ups)
- [ ] Breadboard or custom PCB
- [ ] Jumper wires or ribbon cables
- [ ] 5V 2.5A power supply (USB-C for RPi Zero 2)
- [ ] Micro-USB cable (for RP2040 programming)

### Software Required

- [ ] PiCorePlayer image (latest version)
- [ ] PlatformIO (for firmware compilation)
- [ ] Python 3.6+ (included in PiCorePlayer)
- [ ] i2c-tools (for testing)

### Tools Required

- [ ] Soldering iron and solder
- [ ] Multimeter
- [ ] Wire strippers
- [ ] Small screwdrivers
- [ ] Computer with USB port
- [ ] SD card (8GB minimum) for PiCorePlayer

## Hardware Assembly

### Step 1: Prepare the RP2040 (Raspberry Pi Pico)

1. **Solder headers** (if not pre-soldered):
   - Solder 2x20 pin headers to the Pico board
   - Ensure pins are straight and fully soldered

2. **Test the board**:
   - Connect Pico to computer via micro-USB
   - Verify it shows up as USB device
   - LED should blink or show activity

### Step 2: Wire I2C Connection

Connect RP2040 to RPi Zero 2:

```
RP2040 Pin     │ RPi Zero 2 Pin
───────────────┼─────────────────
GP2 (I2C SDA) │ GPIO2 (Pin 3)
GP3 (I2C SCL) │ GPIO3 (Pin 5)
GND           │ GND (Pin 6)
```

**Add I2C pull-up resistors:**
- 4.7kΩ from SDA to 3.3V
- 4.7kΩ from SCL to 3.3V

### Step 3: Connect Motor Drivers

#### DRV8833 #1 (VU Meters)

```
RP2040        │ DRV8833 #1
──────────────┼────────────
GP0           │ IN1 (Ch A)
GP1           │ IN2 (Ch A)
GP4           │ IN3 (Ch B)
GP5           │ IN4 (Ch B)
3.3V          │ VCC
GND           │ GND
(Motor VM)    │ VM (3.3V-5V)
```

**VU Meter Connections:**
```
DRV8833 OUT1 ──→ Left VU Meter (+)
DRV8833 OUT2 ──→ Left VU Meter (-)
DRV8833 OUT3 ──→ Right VU Meter (+)
DRV8833 OUT4 ──→ Right VU Meter (-)
```

#### DRV8822 (Backlight)

```
RP2040        │ DRV8822
──────────────┼────────────
GP6           │ IN1
GP7           │ IN2
3.3V          │ VCC
GND           │ GND
(Backlight VM)│ VM (5V-12V)
```

**Backlight Connection:**
```
DRV8822 OUT1 ──→ Backlight (+)
DRV8822 OUT2 ──→ Backlight (-)
```

#### DRV8833 #2 (Tape Motor)

```
RP2040        │ DRV8833 #2
──────────────┼────────────
GP8           │ IN1 (Ch A)
GP9           │ IN2 (Ch A)
3.3V          │ VCC
GND           │ GND
5V            │ VM
```

**Tape Motor Connection:**
```
DRV8833 OUT1 ──→ Tape Motor (+)
DRV8833 OUT2 ──→ Tape Motor (-)
```

### Step 4: Connect Digital Inputs

Wire 12 buttons/switches to RP2040:

```
Button 1  ──→ GP13 ──┐
Button 2  ──→ GP14   │
Button 3  ──→ GP15   │
Button 4  ──→ GP16   ├── Common GND
Button 5  ──→ GP17   │
Button 6  ──→ GP18   │
Button 7  ──→ GP19   │
Button 8  ──→ GP20   │
Button 9  ──→ GP21   │
Button 10 ──→ GP22   │
Button 11 ──→ GP26   │
Button 12 ──→ GP27 ──┘
```

**Note:** Internal pull-ups are enabled in firmware. Buttons should connect GPIO to GND when pressed.

### Step 5: Connect Rotary Encoder

```
Encoder Pin   │ RP2040 Pin
──────────────┼────────────
A (CLK)       │ GP10
B (DT)        │ GP11
SW (Button)   │ GP12
GND (COM)     │ GND
```

### Step 6: Power Supply Connections

1. **RP2040 Power:**
   - Can be powered via USB during development
   - For production, connect 3.3V to VSYS and GND

2. **RPi Zero 2 Power:**
   - Use 5V 2.5A USB-C power supply

3. **Motor Driver Power:**
   - DRV8833 VCC: 3.3V (from RP2040 or RPi)
   - DRV8833 VM: 3.3V-5V for VU meters and tape motor
   - DRV8822 VM: 5V-12V for backlight (depending on backlight specs)

### Step 7: Add Decoupling Capacitors

**Critical for stability:**

- RP2040 VCC: 100nF + 10µF near power pins
- Each DRV8833 VCC: 100nF
- Each DRV8833 VM: 100µF electrolytic + 100nF ceramic
- DRV8822 VCC: 100nF
- DRV8822 VM: 220µF electrolytic + 100nF ceramic

### Step 8: Verify Connections

Use a multimeter to verify:

- [ ] No shorts between power and ground
- [ ] Continuity on all signal connections
- [ ] Correct voltage at all power pins
- [ ] I2C pull-ups present on SDA and SCL

## Firmware Installation

### Method 1: Using PlatformIO (Recommended)

#### Install PlatformIO

```bash
# On your development computer (not RPi)
pip install platformio

# Or install PlatformIO IDE (VSCode extension)
# https://platformio.org/install/ide?install=vscode
```

#### Build and Upload Firmware

```bash
# Clone repository
git clone https://github.com/yourusername/roll-streamer.git
cd roll-streamer/firmware

# Build firmware
pio run

# Connect RP2040 via USB and upload
pio run --target upload

# Monitor serial output (optional)
pio device monitor
```

#### Troubleshooting Upload

If upload fails:

```bash
# List available serial ports
pio device list

# Upload to specific port
pio run --target upload --upload-port /dev/ttyACM0

# Or use UF2 bootloader method (see below)
```

### Method 2: Using UF2 Bootloader

#### Build UF2 File

```bash
cd roll-streamer/firmware
pio run
# UF2 file will be in .pio/build/pico/firmware.uf2
```

#### Upload via Bootloader

1. **Enter bootloader mode:**
   - Disconnect RP2040 from USB
   - Hold BOOTSEL button on Pico
   - Connect USB while holding BOOTSEL
   - Release BOOTSEL button
   - Pico should appear as USB drive "RPI-RP2"

2. **Copy firmware:**
   ```bash
   # On Linux/Mac
   cp .pio/build/pico/firmware.uf2 /media/RPI-RP2/

   # On Windows
   # Copy firmware.uf2 to RPI-RP2 drive in File Explorer
   ```

3. **Verify:**
   - RP2040 will automatically reboot
   - Onboard LED should blink
   - Device should be ready

### Verify Firmware

After upload, check that firmware is running:

```bash
# Connect to serial monitor
pio device monitor

# Should see output:
# PiCorePlayer Music Streamer - RP2040 Controller
# Firmware v1.0.0
# Initialization complete
# I2C Address: 0x42
```

## PiCorePlayer Setup

### Install PiCorePlayer

1. **Download PiCorePlayer image:**
   - Visit: https://www.picoreplayer.org/
   - Download latest version (pCP 8.x or newer)

2. **Flash to SD card:**
   ```bash
   # Using dd (Linux/Mac)
   sudo dd if=piCorePlayer*.img of=/dev/sdX bs=4M status=progress

   # Or use Etcher (all platforms)
   # https://www.balena.io/etcher/
   ```

3. **Boot RPi Zero 2:**
   - Insert SD card
   - Connect Ethernet or configure WiFi
   - Power on

4. **Access web interface:**
   - Open browser to: http://picoreplayer.local
   - Or find IP address and navigate to it

### Enable I2C

1. **Via web interface:**
   - Navigate to: Main Page → Tweaks
   - Enable I2C: Yes
   - Save and reboot

2. **Via command line:**
   ```bash
   ssh tc@picoreplayer.local
   # Password: piCore

   sudo vi /opt/bootlocal.sh
   # Add line:
   modprobe i2c-dev

   sudo filetool.sh -b
   sudo reboot
   ```

### Configure ES9038Q2M DAC

1. **Enable I2S output:**
   - Main Page → Squeezelite Settings
   - Audio output: I2S
   - Output device: ES9038Q2M (or appropriate driver)
   - Save

2. **Test audio output:**
   ```bash
   speaker-test -c2 -t wav
   ```

## Software Installation

### Install Dependencies

```bash
ssh tc@picoreplayer.local

# Install required packages
tce-load -wi python3.6.tcz
tce-load -wi i2c-tools.tcz
tce-load -wi python3.6-pip.tcz

# Install Python libraries
sudo pip3 install smbus2

# Make persistent
sudo filetool.sh -b
```

### Install Extension Files

```bash
# Create installation directory
sudo mkdir -p /opt/roll-streamer

# Copy files (from your development machine)
# Method 1: Using SCP
scp -r picore-extension/src tc@picoreplayer.local:/tmp/
scp -r picore-extension/scripts tc@picoreplayer.local:/tmp/
scp -r picore-extension/services tc@picoreplayer.local:/tmp/

# On PiCorePlayer:
sudo cp -r /tmp/src /opt/roll-streamer/
sudo cp -r /tmp/scripts /opt/roll-streamer/
sudo chmod +x /opt/roll-streamer/scripts/*.py

# Method 2: Using git (if available)
cd /opt
sudo git clone https://github.com/yourusername/roll-streamer.git
sudo chmod +x /opt/roll-streamer/picore-extension/scripts/*.py
```

### Install Systemd Services

```bash
# Copy service files
sudo cp /tmp/services/*.service /etc/systemd/system/

# Or if using git clone:
sudo cp /opt/roll-streamer/picore-extension/services/*.service /etc/systemd/system/

# Reload systemd
sudo systemctl daemon-reload

# Enable services
sudo systemctl enable vu-meter.service
sudo systemctl enable input-handler.service

# Make persistent
sudo filetool.sh -b
```

## Configuration

### Test I2C Communication

```bash
# Scan for I2C devices
i2cdetect -y 1

# Expected output:
#      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
# 00:          -- -- -- -- -- -- -- -- -- -- -- -- --
# 10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
# 20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
# 30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
# 40: -- -- 42 -- -- -- -- -- -- -- -- -- -- -- -- --
# 50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
# 60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
# 70: -- -- -- -- -- -- -- --

# Read device ID (should be 0x52)
i2cget -y 1 0x42 0x00
```

### Test RP2040 Controller

```bash
# Test Python library
python3 /opt/roll-streamer/picore-extension/src/rp2040_controller.py

# Should output:
# RP2040 Controller Test
# ----------------------------------------
# Firmware version: 1.0.0
# Sweeping VU meters...
# ...
```

### Configure Input Mappings

Edit `/opt/roll-streamer/picore-extension/scripts/input_handler.py`:

```python
# Modify DEFAULT_INPUT_MAP to customize button functions
DEFAULT_INPUT_MAP = {
    1: ("Play/Pause", SqueezeliteControl.play_pause),
    2: ("Stop", SqueezeliteControl.stop),
    # ... customize as needed
}
```

### Configure VU Meter Settings

Edit `/opt/roll-streamer/picore-extension/scripts/vu_meter_daemon.py`:

```python
# Adjust VU meter calibration
daemon = VUMeterDaemon(
    pipe_path="/tmp/vu_meter_data",
    update_rate=50,         # Hz
    min_db=-20.0,          # Minimum dB level
    max_db=3.0             # Maximum dB level
)
```

## Testing

### Test VU Meters

```bash
# Method 1: Using test mode
python3 /opt/roll-streamer/picore-extension/scripts/vu_meter_daemon.py --test

# Method 2: Manual I2C commands
i2cset -y 1 0x42 0x10 0x09  # Enable VU meters
i2cset -y 1 0x42 0x20 128   # Left VU to 50%
i2cset -y 1 0x42 0x21 192   # Right VU to 75%

# Method 3: Using Python
python3 << EOF
from rp2040_controller import RP2040Controller
import time
with RP2040Controller() as c:
    c.enable_vu_meters(True)
    for i in range(256):
        c.set_vu_meters(i, i)
        time.sleep(0.01)
EOF
```

### Test Digital Inputs

```bash
# Monitor inputs
python3 << EOF
from rp2040_controller import RP2040Controller
import time
with RP2040Controller() as c:
    print("Press buttons to test...")
    while True:
        inputs = c.get_digital_inputs()
        pressed = [i+1 for i, state in enumerate(inputs) if state]
        if pressed:
            print(f"Pressed: {pressed}")
        time.sleep(0.1)
EOF
```

### Test Rotary Encoder

```bash
# Monitor encoder
python3 << EOF
from rp2040_controller import RP2040Controller
import time
with RP2040Controller() as c:
    print("Rotate encoder to test...")
    last_pos = 0
    while True:
        pos = c.get_encoder_position()
        if pos != last_pos:
            print(f"Encoder: {pos}")
            last_pos = pos
        button = c.get_encoder_button()
        if button:
            print(f"Button: {button}")
        time.sleep(0.1)
EOF
```

### Start Services

```bash
# Start services
sudo systemctl start vu-meter.service
sudo systemctl start input-handler.service

# Check status
sudo systemctl status vu-meter.service
sudo systemctl status input-handler.service

# View logs
journalctl -u vu-meter.service -f
journalctl -u input-handler.service -f
```

## Troubleshooting

### I2C Device Not Found

**Problem:** `i2cdetect` doesn't show device at 0x42

**Solutions:**
1. Check I2C is enabled: `lsmod | grep i2c`
2. Verify wiring: SDA, SCL, GND
3. Check pull-up resistors (4.7kΩ on SDA and SCL)
4. Verify RP2040 firmware is running (check serial output)
5. Try different I2C address in firmware (change `I2C_SLAVE_ADDRESS`)

### VU Meters Not Moving

**Problem:** VU meters don't respond to audio

**Solutions:**
1. Check motor driver power supplies
2. Verify PWM signals with oscilloscope
3. Test manually: `i2cset -y 1 0x42 0x20 255`
4. Check DRV8833 connections
5. Verify VU meter polarity
6. Check if VU meters are enabled: `i2cget -y 1 0x42 0x11`

### Digital Inputs Not Working

**Problem:** Button presses not detected

**Solutions:**
1. Verify buttons connect to GND when pressed
2. Check internal pull-ups are enabled in firmware
3. Test with multimeter: measure voltage at GPIO pin
4. Check for proper debouncing
5. Monitor with: `i2cget -y 1 0x42 0x50`

### Services Won't Start

**Problem:** systemd services fail to start

**Solutions:**
```bash
# Check service status
sudo systemctl status vu-meter.service

# View detailed logs
journalctl -xe

# Check Python dependencies
python3 -c "import smbus2; print('OK')"

# Verify file permissions
ls -l /opt/roll-streamer/scripts/

# Check I2C permissions
ls -l /dev/i2c-1

# Add user to i2c group
sudo adduser tc i2c
```

### Audio Pipe Not Found

**Problem:** `/tmp/vu_meter_data` doesn't exist

**Solutions:**
1. Configure ALSA to create named pipe
2. Use PeppyALSA plugin (see VUMeters.md)
3. Create pipe manually: `mkfifo /tmp/vu_meter_data`
4. Verify Squeezelite is playing audio
5. Check ALSA configuration in `/etc/asound.conf`

## Next Steps

1. **Calibrate VU meters** to match your specific meters
2. **Customize input mappings** for your use case
3. **Add case/enclosure** for finished look
4. **Configure startup behavior** (backlight, default volume, etc.)
5. **Add web interface** for remote control (optional)

## Additional Resources

- [I2C Register Map](../docs/hardware/I2C_Register_Map.md)
- [GPIO Pin Allocation](../docs/hardware/GPIO_Allocation.md)
- [Wiring Diagram](../docs/hardware/Wiring_Diagram.md)
- [PiCorePlayer Documentation](https://docs.picoreplayer.org/)
- [RP2040 Datasheet](https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf)

## Support

For issues and questions:
- Check documentation in `docs/` directory
- Review examples and test scripts
- Open issue on GitHub
- PiCorePlayer forum: https://forums.slimdevices.com/

---

**Installation complete! Enjoy your music streamer! 🎵**
