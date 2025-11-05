# I2C Register Map Documentation

## Overview

The RP2040 peripheral controller operates as an I2C slave device at address **0x42** (7-bit addressing). The RPi Zero 2 acts as the I2C master and communicates with the RP2040 to control outputs and read inputs.

## I2C Configuration

- **I2C Address**: 0x42 (7-bit)
- **I2C Speed**: Up to 400 kHz (Fast Mode)
- **Bus**: I2C1 on RP2040 (GP2=SDA, GP3=SCL)
- **Pull-ups**: 4.7kΩ recommended on SDA and SCL lines

## Register Access Protocol

### Write Operation
To write to a register:
```
START → I2C_ADDR(W) → REG_ADDR → DATA_BYTE(S) → STOP
```

Example: Set left VU meter to 50%
```python
# Python using smbus2
bus.write_byte_data(0x42, 0x20, 128)  # REG_VU_LEFT = 128 (50%)
```

### Read Operation
To read from a register:
```
START → I2C_ADDR(W) → REG_ADDR → RESTART → I2C_ADDR(R) → DATA_BYTE(S) → STOP
```

Example: Read digital input status
```python
# Python using smbus2
status = bus.read_byte_data(0x42, 0x50)  # REG_INPUT_STATUS_LOW
```

### Multi-byte Read
To read multiple consecutive registers:
```python
# Python using smbus2
data = bus.read_i2c_block_data(0x42, 0x20, 2)  # Read VU left and right
vu_left = data[0]
vu_right = data[1]
```

## Complete Register Map

| Address | Name | Access | Description | Default |
|---------|------|--------|-------------|---------|
| **Device Information** |
| 0x00 | DEVICE_ID | R | Device identification (0x52) | 0x52 |
| 0x01 | FIRMWARE_VER_MAJ | R | Firmware version major | 1 |
| 0x02 | FIRMWARE_VER_MIN | R | Firmware version minor | 0 |
| 0x03 | FIRMWARE_VER_PATCH | R | Firmware version patch | 0 |
| **Control and Status** |
| 0x10 | CONTROL | R/W | Control register | 0x00 |
| 0x11 | STATUS | R | Status register | 0x01 |
| 0x12 | ERROR | R | Error register | 0x00 |
| **VU Meters** |
| 0x20 | VU_LEFT | R/W | Left VU meter level (0-255) | 0 |
| 0x21 | VU_RIGHT | R/W | Right VU meter level (0-255) | 0 |
| 0x22 | VU_MODE | R/W | VU meter mode | 0x00 |
| **Backlight** |
| 0x30 | BACKLIGHT | R/W | Backlight brightness (0-255) | 128 |
| 0x31 | BACKLIGHT_MODE | R/W | Backlight mode | 0x00 |
| **Tape Motor** |
| 0x40 | TAPE_SPEED | R/W | Tape motor speed (0-255) | 0 |
| 0x41 | TAPE_DIRECTION | R/W | Tape motor direction | 0x00 |
| 0x42 | TAPE_MODE | R/W | Tape motor mode | 0x00 |
| **Digital Inputs** |
| 0x50 | INPUT_STATUS_LOW | R | Digital inputs 1-8 status | 0xFF |
| 0x51 | INPUT_STATUS_HIGH | R | Digital inputs 9-12 status (bits 0-3) | 0x0F |
| 0x52 | INPUT_CHANGED_LOW | R | Inputs 1-8 changed flags | 0x00 |
| 0x53 | INPUT_CHANGED_HIGH | R | Inputs 9-12 changed flags | 0x00 |
| **Rotary Encoder** |
| 0x60 | ENCODER_POS_LOW | R | Encoder position low byte | 0x00 |
| 0x61 | ENCODER_POS_HIGH | R | Encoder position high byte | 0x00 |
| 0x62 | ENCODER_DELTA | R | Encoder change since last read (signed) | 0x00 |
| 0x63 | ENCODER_BUTTON | R | Encoder button status | 0x00 |
| **Configuration** |
| 0x70 | CONFIG_VU_FREQ | R/W | VU meter PWM frequency divider | 0x01 |
| 0x71 | CONFIG_DEBOUNCE | R/W | Input debounce time (ms) | 50 |
| 0x72 | CONFIG_OPTIONS | R/W | Configuration options | 0x00 |
| **Commands** |
| 0xF0 | COMMAND | W | Command register | - |

## Register Details

### Device Information (0x00-0x03)

#### REG_DEVICE_ID (0x00) - Read Only
Device identification constant. Always returns 0x52 ('R' for RP2040).

```python
device_id = bus.read_byte_data(0x42, 0x00)
if device_id == 0x52:
    print("RP2040 Controller detected")
```

#### REG_FIRMWARE_VER_* (0x01-0x03) - Read Only
Firmware version in semantic versioning format (major.minor.patch).

```python
major = bus.read_byte_data(0x42, 0x01)
minor = bus.read_byte_data(0x42, 0x02)
patch = bus.read_byte_data(0x42, 0x03)
print(f"Firmware version: {major}.{minor}.{patch}")
```

### Control and Status (0x10-0x12)

#### REG_CONTROL (0x10) - Read/Write
Control register with bit flags:

| Bit | Name | Description |
|-----|------|-------------|
| 0 | ENABLE | Global enable (0=disabled, 1=enabled) |
| 1 | RESET_ENCODER | Reset encoder position to 0 (write 1) |
| 2 | CLEAR_INPUTS | Clear input changed flags (write 1) |
| 3 | VU_ENABLE | Enable VU meters (0=off, 1=on) |
| 4 | BACKLIGHT_ENABLE | Enable backlight (0=off, 1=on) |
| 5 | TAPE_ENABLE | Enable tape motor (0=off, 1=on) |
| 6 | SAVE_CONFIG | Save config to EEPROM (write 1) |
| 7 | LOAD_CONFIG | Load config from EEPROM (write 1) |

```python
# Enable VU meters and backlight
control = 0x01 | 0x08 | 0x10  # ENABLE | VU_ENABLE | BACKLIGHT_ENABLE
bus.write_byte_data(0x42, 0x10, control)
```

#### REG_STATUS (0x11) - Read Only
Status register with bit flags:

| Bit | Name | Description |
|-----|------|-------------|
| 0 | READY | Device ready (1=ready) |
| 1 | ERROR | Error flag (1=error occurred) |
| 2 | VU_ACTIVE | VU meters are active |
| 3 | BACKLIGHT_ON | Backlight is on |
| 4 | TAPE_RUNNING | Tape motor is running |
| 5 | INPUT_CHANGED | Digital input state changed |
| 6 | ENCODER_CHANGED | Encoder position changed |
| 7 | BUTTON_PRESSED | Encoder button is pressed |

```python
status = bus.read_byte_data(0x42, 0x11)
if status & 0x20:  # INPUT_CHANGED
    print("Digital input changed!")
```

#### REG_ERROR (0x12) - Read Only
Error register with bit flags:

| Bit | Name | Description |
|-----|------|-------------|
| 0 | I2C_OVERFLOW | I2C buffer overflow |
| 1 | INVALID_REG | Invalid register access attempted |
| 2 | INVALID_CMD | Invalid command |
| 3 | PWM_FAULT | PWM generation fault |
| 4 | WATCHDOG | Watchdog timeout occurred |

```python
error = bus.read_byte_data(0x42, 0x12)
if error != 0:
    print(f"Error flags: 0x{error:02X}")
```

### VU Meters (0x20-0x22)

#### REG_VU_LEFT (0x20) - Read/Write
Left VU meter level: 0-255 (0=0%, 255=100%)

```python
# Set left VU to 50%
bus.write_byte_data(0x42, 0x20, 128)
```

#### REG_VU_RIGHT (0x21) - Read/Write
Right VU meter level: 0-255 (0=0%, 255=100%)

```python
# Set right VU to 75%
bus.write_byte_data(0x42, 0x21, 192)
```

#### REG_VU_MODE (0x22) - Read/Write
VU meter operating mode:

| Value | Mode | Description |
|-------|------|-------------|
| 0x00 | NORMAL | Normal VU meter operation |
| 0x01 | PEAK_HOLD | Peak hold mode (hold max value) |
| 0x02 | TEST | Test pattern (sweep) |
| 0xFF | OFF | VU meters disabled |

```python
# Set VU meters to peak hold mode
bus.write_byte_data(0x42, 0x22, 0x01)
```

### Backlight (0x30-0x31)

#### REG_BACKLIGHT (0x30) - Read/Write
Backlight brightness: 0-255 (0=off, 255=maximum brightness)

```python
# Set backlight to 50%
bus.write_byte_data(0x42, 0x30, 128)
```

#### REG_BACKLIGHT_MODE (0x31) - Read/Write
Backlight operating mode:

| Value | Mode | Description |
|-------|------|-------------|
| 0x00 | MANUAL | Manual brightness control |
| 0x01 | AUTO | Auto brightness based on VU levels |
| 0x02 | PULSE | Pulsing effect |
| 0xFF | OFF | Backlight disabled |

```python
# Enable auto brightness mode
bus.write_byte_data(0x42, 0x31, 0x01)
```

### Tape Motor (0x40-0x42)

#### REG_TAPE_SPEED (0x40) - Read/Write
Tape motor speed: 0-255 (0=stopped, 255=maximum speed)

```python
# Set motor speed to 30%
bus.write_byte_data(0x42, 0x40, 77)
```

#### REG_TAPE_DIRECTION (0x41) - Read/Write
Tape motor direction:

| Value | Direction | Description |
|-------|-----------|-------------|
| 0x00 | STOP | Motor stopped (coast) |
| 0x01 | FORWARD | Forward direction |
| 0x02 | REVERSE | Reverse direction |
| 0x03 | BRAKE | Active brake |

```python
# Set motor to forward direction
bus.write_byte_data(0x42, 0x41, 0x01)
```

#### REG_TAPE_MODE (0x42) - Read/Write
Tape motor operating mode:

| Value | Mode | Description |
|-------|------|-------------|
| 0x00 | MANUAL | Manual speed control |
| 0x01 | AUTO | Auto speed based on playback |
| 0xFF | OFF | Motor disabled |

### Digital Inputs (0x50-0x53)

#### REG_INPUT_STATUS_LOW (0x50) - Read Only
Digital inputs 1-8 status (bit 0 = input 1, bit 7 = input 8)
- 0 = Input active (button pressed)
- 1 = Input inactive (button released)

```python
status_low = bus.read_byte_data(0x42, 0x50)
input1_pressed = (status_low & 0x01) == 0
input8_pressed = (status_low & 0x80) == 0
```

#### REG_INPUT_STATUS_HIGH (0x51) - Read Only
Digital inputs 9-12 status (bit 0 = input 9, bit 3 = input 12)
- Bits 4-7 are reserved (read as 1)

```python
status_high = bus.read_byte_data(0x42, 0x51)
input9_pressed = (status_high & 0x01) == 0
input12_pressed = (status_high & 0x08) == 0
```

#### REG_INPUT_CHANGED_LOW/HIGH (0x52-0x53) - Read Only
Change detection flags for digital inputs. Bit is set to 1 when corresponding input changes state.
- Automatically cleared when read
- Can also be cleared by writing CLEAR_INPUTS to control register

```python
changed_low = bus.read_byte_data(0x42, 0x52)
if changed_low & 0x01:
    print("Input 1 changed state")
```

### Rotary Encoder (0x60-0x63)

#### REG_ENCODER_POS_LOW/HIGH (0x60-0x61) - Read Only
16-bit signed encoder position (LOW byte first, little-endian)
- Increments on clockwise rotation
- Decrements on counter-clockwise rotation

```python
# Read 16-bit encoder position
pos_low = bus.read_byte_data(0x42, 0x60)
pos_high = bus.read_byte_data(0x42, 0x61)
position = (pos_high << 8) | pos_low
# Convert to signed 16-bit
if position > 32767:
    position -= 65536
print(f"Encoder position: {position}")
```

#### REG_ENCODER_DELTA (0x62) - Read Only
Signed change in encoder position since last read (-128 to +127)
- Automatically cleared after reading
- Useful for relative positioning

```python
delta = bus.read_byte_data(0x42, 0x62)
# Convert to signed 8-bit
if delta > 127:
    delta -= 256
print(f"Encoder moved: {delta}")
```

#### REG_ENCODER_BUTTON (0x63) - Read Only
Encoder button status:

| Value | Status | Description |
|-------|--------|-------------|
| 0x00 | RELEASED | Button not pressed |
| 0x01 | PRESSED | Button pressed |
| 0x02 | HELD | Button held (>1 second) |
| 0x03 | DOUBLE_CLICK | Double-click detected |

```python
button = bus.read_byte_data(0x42, 0x63)
if button == 0x01:
    print("Button pressed")
elif button == 0x02:
    print("Button held")
```

### Configuration (0x70-0x72)

#### REG_CONFIG_VU_FREQ (0x70) - Read/Write
VU meter PWM frequency divider (1-255)
- Actual frequency = 1000 Hz / divider
- Default: 1 (1000 Hz)

```python
# Set PWM frequency to 500 Hz
bus.write_byte_data(0x42, 0x70, 2)
```

#### REG_CONFIG_DEBOUNCE (0x71) - Read/Write
Input debounce time in milliseconds (1-255)
- Default: 50 ms

```python
# Set debounce to 30 ms
bus.write_byte_data(0x42, 0x71, 30)
```

#### REG_CONFIG_OPTIONS (0x72) - Read/Write
Configuration option flags (reserved for future use)

### Command Register (0xF0)

#### REG_COMMAND (0xF0) - Write Only
Execute special commands:

| Command | Code | Description |
|---------|------|-------------|
| NOP | 0x00 | No operation |
| RESET | 0x01 | Soft reset device |
| FACTORY_RESET | 0x02 | Factory reset (clear EEPROM) |
| CALIBRATE_VU | 0x10 | Calibrate VU meters |
| TEST_VU_LEFT | 0x11 | Test left VU (sweep pattern) |
| TEST_VU_RIGHT | 0x12 | Test right VU (sweep pattern) |
| TEST_VU_BOTH | 0x13 | Test both VU meters |
| TEST_BACKLIGHT | 0x20 | Test backlight (fade in/out) |
| TEST_TAPE_MOTOR | 0x30 | Test tape motor |
| TEST_ALL | 0xFF | Test all outputs |

```python
# Test both VU meters
bus.write_byte_data(0x42, 0xF0, 0x13)
```

## Example Usage

### Python Library Example

```python
#!/usr/bin/env python3
from smbus2 import SMBus
import time

class RP2040Controller:
    def __init__(self, bus=1, address=0x42):
        self.bus = SMBus(bus)
        self.addr = address

    def set_vu_meters(self, left, right):
        """Set VU meter levels (0-255)"""
        self.bus.write_byte_data(self.addr, 0x20, left)
        self.bus.write_byte_data(self.addr, 0x21, right)

    def set_backlight(self, brightness):
        """Set backlight brightness (0-255)"""
        self.bus.write_byte_data(self.addr, 0x30, brightness)

    def get_digital_inputs(self):
        """Read all digital input states"""
        low = self.bus.read_byte_data(self.addr, 0x50)
        high = self.bus.read_byte_data(self.addr, 0x51)
        # Convert to list of booleans (True = pressed)
        inputs = []
        for i in range(8):
            inputs.append((low & (1 << i)) == 0)
        for i in range(4):
            inputs.append((high & (1 << i)) == 0)
        return inputs

    def get_encoder_position(self):
        """Read encoder position"""
        low = self.bus.read_byte_data(self.addr, 0x60)
        high = self.bus.read_byte_data(self.addr, 0x61)
        pos = (high << 8) | low
        if pos > 32767:
            pos -= 65536
        return pos

    def reset_encoder(self):
        """Reset encoder to zero"""
        control = self.bus.read_byte_data(self.addr, 0x10)
        self.bus.write_byte_data(self.addr, 0x10, control | 0x02)

# Usage example
controller = RP2040Controller()

# Set VU meters
controller.set_vu_meters(128, 192)  # Left 50%, Right 75%

# Set backlight
controller.set_backlight(255)  # Full brightness

# Read inputs
inputs = controller.get_digital_inputs()
for i, pressed in enumerate(inputs, 1):
    if pressed:
        print(f"Input {i} is pressed")

# Read encoder
position = controller.get_encoder_position()
print(f"Encoder position: {position}")
```

### Shell Script Example

```bash
#!/bin/bash
# Using i2c-tools

I2C_BUS=1
I2C_ADDR=0x42

# Set left VU meter to 50%
i2cset -y $I2C_BUS $I2C_ADDR 0x20 128

# Set right VU meter to 75%
i2cset -y $I2C_BUS $I2C_ADDR 0x21 192

# Read digital input status
status=$(i2cget -y $I2C_BUS $I2C_ADDR 0x50)
echo "Input status: $status"

# Read encoder position
low=$(i2cget -y $I2C_BUS $I2C_ADDR 0x60)
high=$(i2cget -y $I2C_BUS $I2C_ADDR 0x61)
echo "Encoder position: $((($high << 8) | $low))"
```

## Timing Considerations

### I2C Transaction Timing
- Maximum I2C clock: 400 kHz
- Typical transaction time: <1ms
- Register update rate: 100-1000 Hz recommended

### VU Meter Update Rate
For smooth VU meter operation:
- Minimum update rate: 20 Hz (50ms intervals)
- Recommended rate: 50-100 Hz (10-20ms intervals)
- Maximum rate: 1000 Hz (1ms intervals)

### Input Polling Rate
- Digital inputs: Poll at 10-100 Hz
- Rotary encoder: Poll at 50-200 Hz for smooth response
- Changed flags remain set until read

## Error Handling

### I2C Communication Errors
```python
from smbus2 import SMBus
import smbus2

try:
    bus = SMBus(1)
    data = bus.read_byte_data(0x42, 0x50)
except IOError as e:
    print(f"I2C communication error: {e}")
```

### Device Error Detection
```python
error = bus.read_byte_data(0x42, 0x12)
if error != 0:
    if error & 0x01:
        print("I2C buffer overflow")
    if error & 0x02:
        print("Invalid register access")
    if error & 0x04:
        print("Invalid command")
```

## Troubleshooting

### Device Not Responding
1. Check I2C address using `i2cdetect -y 1`
2. Verify pull-up resistors on SDA/SCL
3. Check 3.3V power supply to RP2040
4. Verify firmware is running (check status LED)

### Registers Return Invalid Data
1. Check device ID register (should be 0x52)
2. Read status register for error flags
3. Send RESET command to clear errors
4. Verify register addresses are correct

### VU Meters Not Responding
1. Check ENABLE and VU_ENABLE bits in control register
2. Verify PWM connections to motor drivers
3. Check motor driver power supplies
4. Use TEST_VU_BOTH command to verify hardware

## Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0 | 2025-11-05 | Initial I2C register map specification |
