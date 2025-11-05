# Wiring Diagram and Connection Guide

## System Overview

```
┌──────────────┐         ┌──────────────┐
│  RPi Zero 2  │◄───I2C──►│   RP2040     │
│ (PiCorePlayer│         │  Controller   │
│   + ES9038)  │         │   (Pico)      │
└──────────────┘         └───────┬───────┘
                                 │
                    ┌────────────┼────────────┐
                    │            │            │
                ┌───▼───┐    ┌───▼───┐    ┌──▼──┐
                │DRV8833│    │DRV8822│    │Input│
                │#1 & #2│    │       │    │ s   │
                └───┬───┘    └───┬───┘    └─────┘
                    │            │
            ┌───────┴─────┐      │
            │             │      │
        ┌───▼───┐    ┌────▼──┐   │
        │VU Left│    │VU Right   │Backlight│
        │ Meter │    │ Meter     │  Panel  │
        └───────┘    └───────┘   └─────────┘
                     Tape Counter
                        Motor
```

## Detailed Connection Tables

### RPi Zero 2 to RP2040 I2C Connection

| RPi Zero 2 | Pin# | Function | RP2040 Pin | Function |
|------------|------|----------|------------|----------|
| GPIO2 | 3 | I2C1 SDA | GP2 | I2C1 SDA |
| GPIO3 | 5 | I2C1 SCL | GP3 | I2C1 SCL |
| GND | 6, 9, 14, 20, 25, 30, 34, 39 | Ground | GND | Ground |
| 3.3V | 1, 17 | Power (optional) | VSYS/3V3 | Power |

**Notes:**
- Add 4.7kΩ pull-up resistors on SDA and SCL lines (typically on RPi side)
- Keep I2C wires short (<30cm recommended)
- Use twisted pair or shielded cable for longer runs
- Common ground is essential

### RP2040 to DRV8833 #1 (VU Meters)

| RP2040 Pin | Function | DRV8833 Pin | Function | Notes |
|------------|----------|-------------|----------|-------|
| GP0 | VU Left IN1 | IN1 | Motor A Input 1 | PWM signal |
| GP1 | VU Left IN2 | IN2 | Motor A Input 2 | Ground (or inverse PWM) |
| GP4 | VU Right IN1 | IN3 | Motor B Input 1 | PWM signal |
| GP5 | VU Right IN2 | IN4 | Motor B Input 2 | Ground (or inverse PWM) |
| GND | Ground | GND | Ground | Common ground |
| 3.3V | Logic power | VCC | Logic supply | 3.3V logic |
| - | - | VM | Motor supply | 3.3V or 5V for VU meters |
| - | - | OUT1 | Motor A+ | To left VU meter (+) |
| - | - | OUT2 | Motor A- | To left VU meter (-) |
| - | - | OUT3 | Motor B+ | To right VU meter (+) |
| - | - | OUT4 | Motor B- | To right VU meter (-) |

**Power:**
- VCC: 3.3V (logic supply from RP2040)
- VM: 3.3V-5V (depending on VU meter specifications)
- Add 100nF ceramic capacitor at VCC
- Add 100µF electrolytic + 100nF ceramic at VM

### RP2040 to DRV8822 (Backlight)

| RP2040 Pin | Function | DRV8822 Pin | Function | Notes |
|------------|----------|-------------|----------|-------|
| GP6 | Backlight IN1 | IN1 | Input 1 | PWM signal |
| GP7 | Backlight IN2 | IN2 | Input 2 | Ground (or inverse PWM) |
| GND | Ground | GND | Ground | Common ground |
| 3.3V | Logic power | VCC | Logic supply | 3.3V logic |
| - | - | VM | Motor supply | 5V-12V for backlight |
| - | - | OUT1 | Output + | To backlight (+) |
| - | - | OUT2 | Output - | To backlight (-) |
| - | - | MODE | Mode select | Tie to GND for fast decay |
| - | - | nFAULT | Fault output | Optional: monitor for faults |
| - | - | nSLEEP | Sleep control | Tie to VCC for normal operation |

**Power:**
- VCC: 3.3V (logic supply)
- VM: 5V-12V (depending on backlight voltage)
- Add 100nF ceramic capacitor at VCC
- Add 220µF electrolytic + 100nF ceramic at VM

### RP2040 to DRV8833 #2 (Tape Counter Motor)

| RP2040 Pin | Function | DRV8833 Pin | Function | Notes |
|------------|----------|-------------|----------|-------|
| GP8 | Tape Motor IN1 | IN1 | Motor A Input 1 | PWM signal |
| GP9 | Tape Motor IN2 | IN2 | Motor A Input 2 | PWM signal (direction control) |
| GND | Ground | GND | Ground | Common ground |
| 3.3V | Logic power | VCC | Logic supply | 3.3V logic |
| - | - | VM | Motor supply | 5V for tape motor |
| - | - | OUT1 | Motor A+ | To tape motor (+) |
| - | - | OUT2 | Motor A- | To tape motor (-) |
| - | - | IN3/IN4 | Motor B (unused) | Can leave floating or tie to GND |
| - | - | OUT3/OUT4 | Motor B (unused) | Leave disconnected |

**Power:**
- VCC: 3.3V (logic supply)
- VM: 5V (typical for small DC motors)
- Add 100nF ceramic capacitor at VCC
- Add 100µF electrolytic + 100nF ceramic at VM

### Digital Inputs (Buttons/Switches)

All 12 digital inputs follow the same connection pattern:

| RP2040 Pin | Input # | Connection | Notes |
|------------|---------|------------|-------|
| GP13 | Input 1 | Switch to GND | Internal pull-up enabled |
| GP14 | Input 2 | Switch to GND | Internal pull-up enabled |
| GP15 | Input 3 | Switch to GND | Internal pull-up enabled |
| GP16 | Input 4 | Switch to GND | Internal pull-up enabled |
| GP17 | Input 5 | Switch to GND | Internal pull-up enabled |
| GP18 | Input 6 | Switch to GND | Internal pull-up enabled |
| GP19 | Input 7 | Switch to GND | Internal pull-up enabled |
| GP20 | Input 8 | Switch to GND | Internal pull-up enabled |
| GP21 | Input 9 | Switch to GND | Internal pull-up enabled |
| GP22 | Input 10 | Switch to GND | Internal pull-up enabled |
| GP26 | Input 11 | Switch to GND | Internal pull-up enabled |
| GP27 | Input 12 | Switch to GND | Internal pull-up enabled |

**Connection Diagram for Each Input:**
```
RP2040 GPIO ──────┬──── Button/Switch ──── GND
                  │
                 Pull-up (internal)
                  │
                3.3V
```

**Optional Hardware Debouncing:**
```
RP2040 GPIO ──────┬──── Button/Switch ──── GND
                  │
                100nF
                  │
                 GND
```

### Rotary Encoder

| RP2040 Pin | Function | Encoder Pin | Notes |
|------------|----------|-------------|-------|
| GP10 | Encoder A | A (or CLK) | Internal pull-up enabled |
| GP11 | Encoder B | B (or DT) | Internal pull-up enabled |
| GP12 | Encoder Button | SW | Internal pull-up enabled |
| GND | Ground | GND (or COM) | Common ground |

**Typical Encoder Pinout (EC11 style):**
```
Looking at encoder from back (solder side):
     ┌─────┐
   A │  1  │
 GND │  2  │
   B │  3  │
     ├─────┤
  SW │  4  │
 GND │  5  │
     └─────┘
```

**Connection:**
```
RP2040 GP10 ──── Encoder Pin 1 (A)
RP2040 GP11 ──── Encoder Pin 3 (B)
RP2040 GP12 ──── Encoder Pin 4 (SW)
GND ──────────── Encoder Pin 2 and Pin 5 (GND)
```

## Complete Wiring Checklist

### Power Supply Connections

- [ ] RPi Zero 2: 5V USB-C power supply (2.5A minimum)
- [ ] RP2040: Powered via USB (during development) or 3.3V from RPi
- [ ] DRV8833 #1 VCC: 3.3V from RP2040 or RPi
- [ ] DRV8833 #1 VM: 3.3V-5V for VU meters
- [ ] DRV8822 VCC: 3.3V from RP2040 or RPi
- [ ] DRV8822 VM: 5V-12V for backlight (separate supply)
- [ ] DRV8833 #2 VCC: 3.3V from RP2040 or RPi
- [ ] DRV8833 #2 VM: 5V for tape motor

### Ground Connections

- [ ] Common ground between all devices
- [ ] RPi GND to RP2040 GND
- [ ] RP2040 GND to all DRV8833/DRV8822 GND
- [ ] All switches/encoder to common GND
- [ ] Motor driver grounds to power supply grounds

### I2C Bus

- [ ] RPi GPIO2 to RP2040 GP2 (SDA)
- [ ] RPi GPIO3 to RP2040 GP3 (SCL)
- [ ] 4.7kΩ pull-up on SDA to 3.3V
- [ ] 4.7kΩ pull-up on SCL to 3.3V

### PWM Outputs

- [ ] GP0 to DRV8833#1 IN1 (VU Left)
- [ ] GP1 to DRV8833#1 IN2 (VU Left)
- [ ] GP4 to DRV8833#1 IN3 (VU Right)
- [ ] GP5 to DRV8833#1 IN4 (VU Right)
- [ ] GP6 to DRV8822 IN1 (Backlight)
- [ ] GP7 to DRV8822 IN2 (Backlight)
- [ ] GP8 to DRV8833#2 IN1 (Tape Motor)
- [ ] GP9 to DRV8833#2 IN2 (Tape Motor)

### Digital Inputs

- [ ] GP13-GP22, GP26-GP27 to switches (12 total)
- [ ] All switches connected to GND when closed

### Rotary Encoder

- [ ] GP10 to Encoder A
- [ ] GP11 to Encoder B
- [ ] GP12 to Encoder Button
- [ ] Encoder GND to common ground

### Decoupling Capacitors

- [ ] 100nF + 10µF at RP2040 VCC
- [ ] 100nF at each DRV8833/DRV8822 VCC
- [ ] 100µF + 100nF at each DRV8833 VM
- [ ] 220µF + 100nF at DRV8822 VM

## PCB Layout Recommendations

### Layer Stack (2-layer PCB)
- **Top Layer**: Signal traces, components
- **Bottom Layer**: Ground plane with power traces

### Critical Trace Routing

1. **I2C Traces**
   - Keep SDA and SCL traces parallel
   - Maximum length: <30cm on PCB
   - Minimum spacing: 0.3mm (12mil)
   - Add ground guard traces if possible

2. **PWM Traces**
   - Route PWM traces away from I2C and encoder signals
   - Keep motor driver outputs away from logic signals
   - Use wider traces (1mm/40mil) for motor outputs

3. **Power Traces**
   - 3.3V rail: 0.5mm (20mil) minimum
   - 5V motor supply: 1.0mm (40mil) minimum
   - Ground: Use ground plane, connect with vias

### Component Placement

1. **RP2040 (Center)**
   - Place in center of board
   - Decoupling caps close to VCC pins
   - I2C connector nearby

2. **Motor Drivers (Periphery)**
   - Place near board edge for cooling
   - Group with output connectors
   - Keep high-current traces short

3. **Connectors**
   - I2C: Near RP2040
   - VU meters: Near DRV8833 #1
   - Backlight: Near DRV8822
   - Tape motor: Near DRV8833 #2
   - Inputs: Group on one edge
   - Encoder: Separate from digital inputs

### Ground Plane Guidelines

- Solid ground plane on bottom layer
- Connect all component grounds to plane
- Use multiple vias (>4) for high-current connections
- Separate analog and digital grounds if possible (join at one point)

## Cable Specifications

### I2C Cable (RPi to RP2040)
- **Type**: 4-conductor cable (SDA, SCL, 3.3V, GND)
- **Length**: <30cm recommended, <1m maximum
- **Connector**: JST-XH 4-pin or similar
- **Wire Gauge**: 26-28 AWG

### Motor Driver to VU Meter Cables
- **Type**: 2-conductor shielded cable
- **Length**: <50cm recommended
- **Connector**: 2-pin terminal block or JST-XH
- **Wire Gauge**: 24-26 AWG

### Motor Driver to Backlight Cable
- **Type**: 2-conductor cable
- **Length**: <50cm recommended
- **Connector**: 2-pin terminal block
- **Wire Gauge**: 22-24 AWG (higher current)

### Digital Input Cables
- **Type**: Multi-conductor ribbon cable or individual wires
- **Length**: <1m
- **Connector**: IDC connector or JST-XH series
- **Wire Gauge**: 26-28 AWG

### Rotary Encoder Cable
- **Type**: 5-conductor shielded cable
- **Length**: <50cm
- **Connector**: 5-pin JST-XH or similar
- **Wire Gauge**: 26-28 AWG

## Connector Pinouts

### J1 - I2C Interface (RPi to RP2040)
```
Pin 1: SDA (I2C Data)
Pin 2: SCL (I2C Clock)
Pin 3: 3.3V (Power)
Pin 4: GND (Ground)
```

### J2 - VU Meter Left
```
Pin 1: VU+ (DRV8833 OUT1)
Pin 2: VU- (DRV8833 OUT2)
```

### J3 - VU Meter Right
```
Pin 1: VU+ (DRV8833 OUT3)
Pin 2: VU- (DRV8833 OUT4)
```

### J4 - Backlight
```
Pin 1: BL+ (DRV8822 OUT1)
Pin 2: BL- (DRV8822 OUT2)
```

### J5 - Tape Motor
```
Pin 1: M+ (DRV8833 OUT1)
Pin 2: M- (DRV8833 OUT2)
```

### J6 - Digital Inputs
```
Pin 1: Input 1 (GP13)
Pin 2: Input 2 (GP14)
...
Pin 12: Input 12 (GP27)
Pin 13: GND (Common)
```

### J7 - Rotary Encoder
```
Pin 1: A (GP10)
Pin 2: B (GP11)
Pin 3: SW (GP12)
Pin 4: GND
Pin 5: GND (shield)
```

## Testing Points

Add test points on PCB for debugging:

- [ ] TP1: 3.3V rail
- [ ] TP2: GND
- [ ] TP3: I2C SDA
- [ ] TP4: I2C SCL
- [ ] TP5: VU Left PWM (GP0)
- [ ] TP6: VU Right PWM (GP4)
- [ ] TP7: Backlight PWM (GP6)
- [ ] TP8: Tape Motor PWM (GP8)

## Assembly Notes

1. **Solder Components in Order:**
   - SMD components first (if any)
   - RP2040 module
   - Decoupling capacitors
   - Motor driver ICs
   - Connectors
   - Test points

2. **Inspection:**
   - Check for solder bridges
   - Verify polarized components
   - Test continuity of power and ground

3. **Initial Power-On:**
   - Connect only 3.3V power initially
   - Measure voltages at test points
   - Check for excessive current draw
   - Load firmware onto RP2040

4. **Functional Testing:**
   - Test I2C communication
   - Test each PWM output
   - Test digital inputs
   - Test encoder
   - Connect motors/VU meters last

## Safety Considerations

- **ESD Protection**: Handle RP2040 with ESD precautions
- **Power Sequencing**: Power up 3.3V before motors
- **Current Limits**: Ensure motor drivers have proper current limiting
- **Thermal**: Provide heatsinking for motor drivers if needed
- **Fusing**: Add fuse on main power input
- **Reverse Polarity**: Add protection diode on power input

## Revision History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2025-11-05 | Initial wiring diagram documentation |
