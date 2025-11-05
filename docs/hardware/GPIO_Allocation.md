# RP2040 GPIO Pin Allocation

## Overview
This document details the complete GPIO pin allocation for the RP2040 peripheral controller in the PiCorePlayer music streamer project.

## Pin Summary Table

| GPIO | Function | Direction | Connection | Notes |
|------|----------|-----------|------------|-------|
| GP0 | VU Left IN1 | Output (PWM) | DRV8833 #1 IN1 | Left VU meter forward |
| GP1 | VU Left IN2 | Output (PWM) | DRV8833 #1 IN2 | Left VU meter reverse |
| GP2 | I2C1 SDA | Bidirectional | RPi Zero 2 GPIO2 | I2C slave data line |
| GP3 | I2C1 SCL | Input | RPi Zero 2 GPIO3 | I2C slave clock line |
| GP4 | VU Right IN1 | Output (PWM) | DRV8833 #1 IN3 | Right VU meter forward |
| GP5 | VU Right IN2 | Output (PWM) | DRV8833 #1 IN4 | Right VU meter reverse |
| GP6 | Backlight IN1 | Output (PWM) | DRV8822 IN1 | Panel backlight forward |
| GP7 | Backlight IN2 | Output (PWM) | DRV8822 IN2 | Panel backlight reverse |
| GP8 | Tape Motor IN1 | Output (PWM) | DRV8833 #2 IN1 | Tape counter forward |
| GP9 | Tape Motor IN2 | Output (PWM) | DRV8833 #2 IN2 | Tape counter reverse |
| GP10 | Encoder A | Input (Pull-up) | Rotary encoder A | Quadrature input A |
| GP11 | Encoder B | Input (Pull-up) | Rotary encoder B | Quadrature input B |
| GP12 | Encoder Button | Input (Pull-up) | Rotary encoder SW | Active low button |
| GP13 | Digital Input 1 | Input (Pull-up) | User button/switch 1 | Active low |
| GP14 | Digital Input 2 | Input (Pull-up) | User button/switch 2 | Active low |
| GP15 | Digital Input 3 | Input (Pull-up) | User button/switch 3 | Active low |
| GP16 | Digital Input 4 | Input (Pull-up) | User button/switch 4 | Active low |
| GP17 | Digital Input 5 | Input (Pull-up) | User button/switch 5 | Active low |
| GP18 | Digital Input 6 | Input (Pull-up) | User button/switch 6 | Active low |
| GP19 | Digital Input 7 | Input (Pull-up) | User button/switch 7 | Active low |
| GP20 | Digital Input 8 | Input (Pull-up) | User button/switch 8 | Active low |
| GP21 | Digital Input 9 | Input (Pull-up) | User button/switch 9 | Active low |
| GP22 | Digital Input 10 | Input (Pull-up) | User button/switch 10 | Active low |
| GP23 | *Reserved* | - | SMPS mode select | Do not use |
| GP24 | *Not available* | - | - | Not on Pico board |
| GP25 | Status LED | Output | Onboard LED | Optional status indicator |
| GP26 | Digital Input 11 | Input (Pull-up) | User button/switch 11 | Active low |
| GP27 | Digital Input 12 | Input (Pull-up) | User button/switch 12 | Active low |
| GP28 | *Available* | ADC | - | Future expansion |
| GP29 | *Available* | ADC | - | Future expansion |

## Detailed Connection Information

### I2C Bus (Communication with RPi Zero 2)

**RP2040 Side:**
- GP2 (I2C1 SDA) ← 4.7kΩ pull-up to 3.3V
- GP3 (I2C1 SCL) ← 4.7kΩ pull-up to 3.3V

**RPi Zero 2 Side:**
- GPIO2 (SDA) - Pin 3 on 40-pin header
- GPIO3 (SCL) - Pin 5 on 40-pin header

**Notes:**
- I2C slave address: 0x42 (7-bit address)
- Both devices operate at 3.3V logic levels
- Maximum I2C bus speed: 400 kHz (Fast Mode)
- Pull-up resistors should be placed on the RPi side or on the RP2040 side (not both)

### VU Meters (DRV8833 Dual H-Bridge)

The DRV8833 can drive two DC motors. We use one DRV8833 to drive both VU meters.

**DRV8833 #1 Connections:**

Left VU Meter:
- RP2040 GP0 → DRV8833 IN1 (Channel 1)
- RP2040 GP1 → DRV8833 IN2 (Channel 1)
- DRV8833 OUT1 → Left VU Meter Coil (+)
- DRV8833 OUT2 → Left VU Meter Coil (-)

Right VU Meter:
- RP2040 GP4 → DRV8833 IN3 (Channel 2)
- RP2040 GP5 → DRV8833 IN4 (Channel 2)
- DRV8833 OUT3 → Right VU Meter Coil (+)
- DRV8833 OUT4 → Right VU Meter Coil (-)

**Power:**
- VM: Connect to appropriate voltage for VU meters (typically 3.3V or 5V)
- VCC: 3.3V logic supply
- GND: Common ground with RP2040

**PWM Settings:**
- Frequency: 1000 Hz (1 kHz)
- Resolution: 8-bit (0-255)
- Mode: Phase/enable or PWM control

**Control Method:**
To drive the meter forward (deflect right):
- IN1 = PWM duty cycle (0-255)
- IN2 = LOW (0)

To brake/stop:
- IN1 = LOW (0)
- IN2 = LOW (0)

For precise control, you can also use:
- IN1 = PWM signal
- IN2 = Inverted PWM signal (for smooth braking)

### Panel Backlight (DRV8822 Single H-Bridge)

The DRV8822 is a more powerful motor driver suitable for higher current loads like panel backlighting.

**DRV8822 Connections:**
- RP2040 GP6 → DRV8822 IN1
- RP2040 GP7 → DRV8822 IN2
- DRV8822 OUT1 → Backlight (+)
- DRV8822 OUT2 → Backlight (-)

**Power:**
- VM: Connect to appropriate voltage for backlight (typically 5V-12V)
- VCC: 3.3V logic supply
- GND: Common ground with RP2040

**PWM Settings:**
- Frequency: 1000 Hz (1 kHz)
- Resolution: 8-bit (0-255)

**Control Method:**
To turn on backlight:
- IN1 = PWM duty cycle (0-255)
- IN2 = LOW (0)

### Tape Counter Motor (DRV8833 Dual H-Bridge)

**DRV8833 #2 Connections:**
- RP2040 GP8 → DRV8833 IN1
- RP2040 GP9 → DRV8833 IN2
- DRV8833 OUT1 → Tape Motor (+)
- DRV8833 OUT2 → Tape Motor (-)

**Power:**
- VM: Connect to appropriate voltage for motor (typically 5V)
- VCC: 3.3V logic supply
- GND: Common ground with RP2040

**PWM Settings:**
- Frequency: 1000 Hz (1 kHz)
- Resolution: 8-bit (0-255)

**Control Method:**
Forward rotation:
- IN1 = PWM duty cycle (0-255)
- IN2 = LOW (0)

Reverse rotation:
- IN1 = LOW (0)
- IN2 = PWM duty cycle (0-255)

Brake:
- IN1 = HIGH (255)
- IN2 = HIGH (255)

### Rotary Encoder

**Encoder Connections:**
- RP2040 GP10 → Encoder Channel A
- RP2040 GP11 → Encoder Channel B
- RP2040 GP12 → Encoder Switch (Button)
- Encoder Common → GND

**Notes:**
- Internal pull-ups enabled on all encoder pins
- Typical encoder type: Incremental quadrature encoder
- Detents: Typically 20-30 per revolution
- Pulses per revolution: 20-30 PPR (depends on encoder type)
- Button: Active low (pressed = LOW, released = HIGH)

**Recommended Encoder Types:**
- EC11 rotary encoder (common, inexpensive)
- Bourns PEC11R series
- Alps EC11 series

### Digital Inputs (12 Channels)

All digital inputs follow the same connection pattern:

**Connection:**
- RP2040 GP13-GP22, GP26-GP27 → Switch/Button → GND
- Internal pull-up resistors enabled

**Logic Levels:**
- Button Released (not pressed): HIGH (3.3V via pull-up)
- Button Pressed: LOW (0V, connected to GND)

**Debouncing:**
- Hardware: Optional 100nF capacitor to GND at each input
- Software: 50ms debounce time implemented in firmware

**Example Digital Input Applications:**
1. Play/Pause button
2. Next track button
3. Previous track button
4. Volume up button
5. Volume down button
6. Power on/off switch
7. Mode select switches
8. Source select buttons
9. Menu navigation
10. Settings buttons
11-12. User-defined functions

## Power Supply Requirements

### RP2040 Power
- Supply voltage: 3.3V
- Current consumption: ~50mA typical, ~150mA max
- Power input: Via USB or external 3.3V regulator

### Motor Drivers
**DRV8833 (x2):**
- Logic supply (VCC): 3.3V @ ~10mA each
- Motor supply (VM): 2.7V-10.8V
- Peak output current: 1.5A per channel
- Continuous current: 1A per channel with proper cooling

**DRV8822:**
- Logic supply (VCC): 3.3V @ ~10mA
- Motor supply (VM): 8V-45V
- Peak output current: 2.5A
- Continuous current: 2A with proper cooling

### Typical Current Draw
- VU Meters: ~10-50mA each (depending on meter type)
- Backlight: 100mA-500mA (depending on type)
- Tape Motor: 50-200mA (depending on load)

**Total System Current (3.3V rail):**
- RP2040: 50-150mA
- Motor driver logic: ~30mA
- Total: ~200mA recommended supply capacity (500mA for safety margin)

**Total System Current (Motor supply rails):**
- Depends heavily on motors and backlight
- Recommend 1A+ supply for VM rails

## PCB Design Considerations

### Trace Widths
- 3.3V power: 0.5mm minimum (20mil)
- Motor driver outputs: 1.0mm minimum (40mil)
- Signal traces: 0.25mm minimum (10mil)
- I2C traces: 0.3mm minimum (12mil), keep short

### Ground Plane
- Use solid ground plane on bottom layer
- Connect all GND pins to ground plane with vias
- Separate analog and digital grounds if possible, connect at single point

### Decoupling Capacitors
- RP2040 VCC: 100nF + 10µF at power pins
- DRV8833 VCC: 100nF at each IC
- DRV8833 VM: 100µF electrolytic + 100nF ceramic at each IC
- DRV8822 VCC: 100nF
- DRV8822 VM: 220µF electrolytic + 100nF ceramic

### Motor Driver Considerations
- Place current sense resistors close to driver ICs
- Use Kelvin connections for accurate current sensing
- Add snubber diodes (if not internal to driver) for inductive loads
- Ensure proper heatsinking for continuous operation

### ESD Protection
- Add ESD protection diodes on all external connectors
- Use TVS diodes on I2C lines if long cable runs

## Firmware Pin Initialization

The firmware automatically configures all pins according to this allocation:

```cpp
// I2C pins configured by Wire1.begin()
pinMode(PIN_I2C_SDA, INPUT_PULLUP);
pinMode(PIN_I2C_SCL, INPUT_PULLUP);

// PWM outputs
pinMode(PIN_VU_LEFT_IN1, OUTPUT);
pinMode(PIN_VU_LEFT_IN2, OUTPUT);
// ... etc for all PWM pins

// Digital inputs with pull-ups
pinMode(PIN_INPUT_1, INPUT_PULLUP);
// ... etc for all input pins

// Encoder with pull-ups
pinMode(PIN_ENCODER_A, INPUT_PULLUP);
pinMode(PIN_ENCODER_B, INPUT_PULLUP);
pinMode(PIN_ENCODER_BTN, INPUT_PULLUP);
```

## Testing Procedure

### 1. Power Supply Test
- Verify 3.3V rail is stable
- Check for excessive current draw
- Measure voltage at RP2040 VCC pins

### 2. I2C Communication Test
- Use i2cdetect on RPi to find device at 0x42
- Send simple read/write commands
- Verify acknowledgment

### 3. PWM Output Test
- Connect oscilloscope to each PWM pin
- Verify 1kHz frequency
- Check duty cycle control (0-100%)

### 4. Digital Input Test
- Pull each input to ground manually
- Verify firmware detects state change
- Check debouncing works correctly

### 5. Encoder Test
- Rotate encoder and verify count changes
- Press button and verify detection
- Test at various rotation speeds

### 6. Motor Driver Test
- Start with no load connected
- Apply low duty cycle PWM
- Verify output voltage with multimeter
- Connect load and test under real conditions

## Troubleshooting

### I2C Not Detected
- Check SDA/SCL connections and pull-up resistors
- Verify 3.3V power to RP2040
- Check I2C address (should be 0x42)
- Verify firmware is running (use status LED)

### PWM Not Working
- Check pin configuration in firmware
- Verify PWM frequency and resolution settings
- Check for conflicts with other peripherals
- Measure output with oscilloscope

### Digital Inputs Always High/Low
- Check pull-up resistor configuration
- Verify switches are connected properly
- Check for shorts to GND or VCC
- Test with multimeter

### Motor Drivers Not Responding
- Verify VM power supply is connected
- Check VCC logic supply is 3.3V
- Ensure GND is common between all devices
- Check for thermal shutdown (overheating)

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-11-05 | Initial | Initial GPIO allocation and documentation |
