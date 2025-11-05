/**
 * @file pin_config.h
 * @brief GPIO Pin Configuration for RP2040 Peripheral Controller
 *
 * PiCorePlayer Music Streamer - RP2040 Pin Assignments
 *
 * Hardware Connections:
 * - I2C slave interface to RPi Zero 2
 * - 2x VU meters via DRV8833 motor drivers
 * - 1x Backlight via DRV8822 motor driver
 * - 1x Tape counter motor via DRV8833
 * - 12x Digital inputs
 * - 1x Rotary encoder with button
 */

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// ============================================================================
// I2C INTERFACE (I2C1)
// ============================================================================
#define PIN_I2C_SDA           2    // I2C1 SDA - Connect to RPi Zero 2 GPIO2 (SDA)
#define PIN_I2C_SCL           3    // I2C1 SCL - Connect to RPi Zero 2 GPIO3 (SCL)
#define I2C_SLAVE_ADDRESS     0x42 // 7-bit I2C slave address

// ============================================================================
// VU METERS - PWM OUTPUTS (DRV8833 Motor Drivers)
// ============================================================================
// Left VU Meter - DRV8833 #1
#define PIN_VU_LEFT_IN1       0    // DRV8833 Input 1 (PWM)
#define PIN_VU_LEFT_IN2       1    // DRV8833 Input 2 (PWM)

// Right VU Meter - DRV8833 #1
#define PIN_VU_RIGHT_IN1      4    // DRV8833 Input 1 (PWM)
#define PIN_VU_RIGHT_IN2      5    // DRV8833 Input 2 (PWM)

// PWM Configuration for VU Meters
#define VU_PWM_FREQUENCY      1000 // 1kHz PWM frequency
#define VU_PWM_RESOLUTION     255  // 8-bit resolution (0-255)

// ============================================================================
// PANEL BACKLIGHT - PWM OUTPUT (DRV8822 Motor Driver)
// ============================================================================
#define PIN_BACKLIGHT_IN1     6    // DRV8822 Input 1 (PWM)
#define PIN_BACKLIGHT_IN2     7    // DRV8822 Input 2 (PWM)

// PWM Configuration for Backlight
#define BACKLIGHT_PWM_FREQUENCY 1000 // 1kHz PWM frequency
#define BACKLIGHT_PWM_RESOLUTION 255 // 8-bit resolution (0-255)

// ============================================================================
// TAPE COUNTER MOTOR - PWM OUTPUT (DRV8833 #2)
// ============================================================================
#define PIN_TAPE_MOTOR_IN1    8    // DRV8833 Input 1 (PWM)
#define PIN_TAPE_MOTOR_IN2    9    // DRV8833 Input 2 (PWM)

// PWM Configuration for Tape Motor
#define TAPE_PWM_FREQUENCY    1000 // 1kHz PWM frequency
#define TAPE_PWM_RESOLUTION   255  // 8-bit resolution (0-255)

// ============================================================================
// ROTARY ENCODER INPUT
// ============================================================================
#define PIN_ENCODER_A         10   // Rotary encoder channel A
#define PIN_ENCODER_B         11   // Rotary encoder channel B
#define PIN_ENCODER_BTN       12   // Rotary encoder button (active low)

// Encoder Configuration
#define ENCODER_DEBOUNCE_MS   5    // Debounce time in milliseconds

// ============================================================================
// DIGITAL INPUTS (12 channels)
// ============================================================================
#define PIN_INPUT_1           13   // Digital input 1 (active low, internal pullup)
#define PIN_INPUT_2           14   // Digital input 2 (active low, internal pullup)
#define PIN_INPUT_3           15   // Digital input 3 (active low, internal pullup)
#define PIN_INPUT_4           16   // Digital input 4 (active low, internal pullup)
#define PIN_INPUT_5           17   // Digital input 5 (active low, internal pullup)
#define PIN_INPUT_6           18   // Digital input 6 (active low, internal pullup)
#define PIN_INPUT_7           19   // Digital input 7 (active low, internal pullup)
#define PIN_INPUT_8           20   // Digital input 8 (active low, internal pullup)
#define PIN_INPUT_9           21   // Digital input 9 (active low, internal pullup)
#define PIN_INPUT_10          22   // Digital input 10 (active low, internal pullup)
#define PIN_INPUT_11          26   // Digital input 11 (active low, internal pullup)
#define PIN_INPUT_12          27   // Digital input 12 (active low, internal pullup)

// Digital Input Configuration
#define NUM_DIGITAL_INPUTS    12
#define INPUT_DEBOUNCE_MS     50   // Debounce time in milliseconds

// Array of all digital input pins for easy iteration
const uint8_t DIGITAL_INPUT_PINS[NUM_DIGITAL_INPUTS] = {
    PIN_INPUT_1, PIN_INPUT_2, PIN_INPUT_3, PIN_INPUT_4,
    PIN_INPUT_5, PIN_INPUT_6, PIN_INPUT_7, PIN_INPUT_8,
    PIN_INPUT_9, PIN_INPUT_10, PIN_INPUT_11, PIN_INPUT_12
};

// ============================================================================
// RESERVED PINS (DO NOT USE)
// ============================================================================
// GP23: Used by RP2040 for SMPS mode (on Pico board)
// GP24: Not available on Pico board
// GP25: Onboard LED (Pico board)
// GP28-29: ADC inputs, can be repurposed if needed

// ============================================================================
// STATUS LED (Optional - uses onboard LED)
// ============================================================================
#define PIN_STATUS_LED        25   // Onboard LED on Pico board

#endif // PIN_CONFIG_H
