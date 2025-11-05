/**
 * @file i2c_registers.h
 * @brief I2C Register Map and Command Interface
 *
 * PiCorePlayer Music Streamer - RP2040 I2C Slave Register Definitions
 *
 * I2C Slave Address: 0x42 (7-bit)
 *
 * Register Access:
 * - Write: Send register address followed by data byte(s)
 * - Read: Send register address, then read data byte(s)
 */

#ifndef I2C_REGISTERS_H
#define I2C_REGISTERS_H

#include <stdint.h>

// ============================================================================
// I2C CONFIGURATION
// ============================================================================
#define I2C_SLAVE_ADDRESS     0x42  // 7-bit I2C address
#define I2C_BUFFER_SIZE       32    // Maximum I2C transaction size

// ============================================================================
// REGISTER MAP
// ============================================================================

// --- Device Information (Read-Only) ---
#define REG_DEVICE_ID         0x00  // Device ID register (R)
#define REG_FIRMWARE_VER_MAJ  0x01  // Firmware version major (R)
#define REG_FIRMWARE_VER_MIN  0x02  // Firmware version minor (R)
#define REG_FIRMWARE_VER_PATCH 0x03 // Firmware version patch (R)

// --- Control and Status ---
#define REG_CONTROL           0x10  // Control register (R/W)
#define REG_STATUS            0x11  // Status register (R)
#define REG_ERROR             0x12  // Error register (R)

// --- VU Meters (PWM Control) ---
#define REG_VU_LEFT           0x20  // Left VU meter level (0-255) (R/W)
#define REG_VU_RIGHT          0x21  // Right VU meter level (0-255) (R/W)
#define REG_VU_MODE           0x22  // VU meter mode (R/W)

// --- Backlight Control ---
#define REG_BACKLIGHT         0x30  // Backlight brightness (0-255) (R/W)
#define REG_BACKLIGHT_MODE    0x31  // Backlight mode (R/W)

// --- Tape Counter Motor Control ---
#define REG_TAPE_SPEED        0x40  // Tape motor speed (0-255) (R/W)
#define REG_TAPE_DIRECTION    0x41  // Tape motor direction (R/W)
#define REG_TAPE_MODE         0x42  // Tape motor mode (R/W)

// --- Digital Inputs (Read-Only) ---
#define REG_INPUT_STATUS_LOW  0x50  // Digital inputs 1-8 status (R)
#define REG_INPUT_STATUS_HIGH 0x51  // Digital inputs 9-12 status (R)
#define REG_INPUT_CHANGED_LOW 0x52  // Digital inputs 1-8 changed flags (R)
#define REG_INPUT_CHANGED_HIGH 0x53 // Digital inputs 9-12 changed flags (R)

// --- Rotary Encoder (Read-Only) ---
#define REG_ENCODER_POS_LOW   0x60  // Encoder position low byte (R)
#define REG_ENCODER_POS_HIGH  0x61  // Encoder position high byte (R)
#define REG_ENCODER_DELTA     0x62  // Encoder delta since last read (R)
#define REG_ENCODER_BUTTON    0x63  // Encoder button status (R)

// --- Configuration Registers ---
#define REG_CONFIG_VU_FREQ    0x70  // VU meter PWM frequency (R/W)
#define REG_CONFIG_DEBOUNCE   0x71  // Input debounce time (R/W)
#define REG_CONFIG_OPTIONS    0x72  // Configuration options (R/W)

// --- Command Register (Write-Only) ---
#define REG_COMMAND           0xF0  // Command register (W)

// ============================================================================
// REGISTER BIT DEFINITIONS
// ============================================================================

// REG_CONTROL (0x10) - Control Register Bits
#define CTRL_ENABLE           (1 << 0)  // Global enable
#define CTRL_RESET_ENCODER    (1 << 1)  // Reset encoder position to 0
#define CTRL_CLEAR_INPUTS     (1 << 2)  // Clear input changed flags
#define CTRL_VU_ENABLE        (1 << 3)  // Enable VU meters
#define CTRL_BACKLIGHT_ENABLE (1 << 4)  // Enable backlight
#define CTRL_TAPE_ENABLE      (1 << 5)  // Enable tape motor
#define CTRL_SAVE_CONFIG      (1 << 6)  // Save config to EEPROM
#define CTRL_LOAD_CONFIG      (1 << 7)  // Load config from EEPROM

// REG_STATUS (0x11) - Status Register Bits
#define STATUS_READY          (1 << 0)  // Device ready
#define STATUS_ERROR          (1 << 1)  // Error flag
#define STATUS_VU_ACTIVE      (1 << 2)  // VU meters active
#define STATUS_BACKLIGHT_ON   (1 << 3)  // Backlight on
#define STATUS_TAPE_RUNNING   (1 << 4)  // Tape motor running
#define STATUS_INPUT_CHANGED  (1 << 5)  // Digital input changed
#define STATUS_ENCODER_CHANGED (1 << 6) // Encoder position changed
#define STATUS_BUTTON_PRESSED (1 << 7)  // Encoder button pressed

// REG_ERROR (0x12) - Error Register Bits
#define ERROR_I2C_OVERFLOW    (1 << 0)  // I2C buffer overflow
#define ERROR_INVALID_REG     (1 << 1)  // Invalid register access
#define ERROR_INVALID_CMD     (1 << 2)  // Invalid command
#define ERROR_PWM_FAULT       (1 << 3)  // PWM generation fault
#define ERROR_WATCHDOG        (1 << 4)  // Watchdog timeout
#define ERROR_RESERVED_5      (1 << 5)  // Reserved
#define ERROR_RESERVED_6      (1 << 6)  // Reserved
#define ERROR_RESERVED_7      (1 << 7)  // Reserved

// REG_VU_MODE (0x22) - VU Meter Mode
#define VU_MODE_NORMAL        0x00      // Normal VU meter operation
#define VU_MODE_PEAK_HOLD     0x01      // Peak hold mode
#define VU_MODE_TEST          0x02      // Test pattern mode
#define VU_MODE_OFF           0xFF      // VU meters off

// REG_BACKLIGHT_MODE (0x31) - Backlight Mode
#define BACKLIGHT_MODE_MANUAL 0x00      // Manual brightness control
#define BACKLIGHT_MODE_AUTO   0x01      // Auto brightness based on VU
#define BACKLIGHT_MODE_PULSE  0x02      // Pulsing effect
#define BACKLIGHT_MODE_OFF    0xFF      // Backlight off

// REG_TAPE_DIRECTION (0x41) - Tape Motor Direction
#define TAPE_DIR_STOP         0x00      // Motor stopped
#define TAPE_DIR_FORWARD      0x01      // Forward direction
#define TAPE_DIR_REVERSE      0x02      // Reverse direction
#define TAPE_DIR_BRAKE        0x03      // Active brake

// REG_TAPE_MODE (0x42) - Tape Motor Mode
#define TAPE_MODE_MANUAL      0x00      // Manual speed control
#define TAPE_MODE_AUTO        0x01      // Auto speed based on playback
#define TAPE_MODE_OFF         0xFF      // Motor off

// REG_ENCODER_BUTTON (0x63) - Encoder Button Status
#define ENC_BTN_RELEASED      0x00      // Button released
#define ENC_BTN_PRESSED       0x01      // Button pressed
#define ENC_BTN_HELD          0x02      // Button held (>1 second)
#define ENC_BTN_DOUBLE_CLICK  0x03      // Double-click detected

// ============================================================================
// COMMAND DEFINITIONS (REG_COMMAND = 0xF0)
// ============================================================================
#define CMD_NOP               0x00  // No operation
#define CMD_RESET             0x01  // Soft reset
#define CMD_FACTORY_RESET     0x02  // Factory reset (clear EEPROM)
#define CMD_CALIBRATE_VU      0x10  // Calibrate VU meters
#define CMD_TEST_VU_LEFT      0x11  // Test left VU meter (sweep)
#define CMD_TEST_VU_RIGHT     0x12  // Test right VU meter (sweep)
#define CMD_TEST_VU_BOTH      0x13  // Test both VU meters
#define CMD_TEST_BACKLIGHT    0x20  // Test backlight (fade in/out)
#define CMD_TEST_TAPE_MOTOR   0x30  // Test tape motor
#define CMD_TEST_ALL          0xFF  // Test all outputs

// ============================================================================
// DEVICE IDENTIFICATION
// ============================================================================
#define DEVICE_ID             0x52  // 'R' for RP2040 Controller
#define FIRMWARE_VERSION_MAJOR 1
#define FIRMWARE_VERSION_MINOR 0
#define FIRMWARE_VERSION_PATCH 0

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @brief I2C Register Bank Structure
 *
 * This structure mirrors the I2C register map in memory
 */
typedef struct {
    // Device Information (Read-Only)
    uint8_t device_id;              // 0x00
    uint8_t fw_ver_major;           // 0x01
    uint8_t fw_ver_minor;           // 0x02
    uint8_t fw_ver_patch;           // 0x03
    uint8_t reserved_04[12];        // 0x04-0x0F

    // Control and Status
    uint8_t control;                // 0x10
    uint8_t status;                 // 0x11
    uint8_t error;                  // 0x12
    uint8_t reserved_13[13];        // 0x13-0x1F

    // VU Meters
    uint8_t vu_left;                // 0x20
    uint8_t vu_right;               // 0x21
    uint8_t vu_mode;                // 0x22
    uint8_t reserved_23[13];        // 0x23-0x2F

    // Backlight
    uint8_t backlight;              // 0x30
    uint8_t backlight_mode;         // 0x31
    uint8_t reserved_32[14];        // 0x32-0x3F

    // Tape Motor
    uint8_t tape_speed;             // 0x40
    uint8_t tape_direction;         // 0x41
    uint8_t tape_mode;              // 0x42
    uint8_t reserved_43[13];        // 0x43-0x4F

    // Digital Inputs
    uint8_t input_status_low;       // 0x50 (bits 0-7 = inputs 1-8)
    uint8_t input_status_high;      // 0x51 (bits 0-3 = inputs 9-12)
    uint8_t input_changed_low;      // 0x52
    uint8_t input_changed_high;     // 0x53
    uint8_t reserved_54[12];        // 0x54-0x5F

    // Rotary Encoder
    uint8_t encoder_pos_low;        // 0x60
    uint8_t encoder_pos_high;       // 0x61
    int8_t  encoder_delta;          // 0x62 (signed)
    uint8_t encoder_button;         // 0x63
    uint8_t reserved_64[12];        // 0x64-0x6F

    // Configuration
    uint8_t config_vu_freq;         // 0x70
    uint8_t config_debounce;        // 0x71
    uint8_t config_options;         // 0x72
    uint8_t reserved_73[13];        // 0x73-0x7F
} __attribute__((packed)) I2CRegisterBank;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

/**
 * @brief Initialize I2C register bank
 */
void i2c_registers_init(void);

/**
 * @brief Read from register
 * @param reg_addr Register address
 * @return Register value
 */
uint8_t i2c_register_read(uint8_t reg_addr);

/**
 * @brief Write to register
 * @param reg_addr Register address
 * @param value Value to write
 * @return true if write successful
 */
bool i2c_register_write(uint8_t reg_addr, uint8_t value);

/**
 * @brief Execute command
 * @param command Command code
 * @return true if command executed successfully
 */
bool i2c_execute_command(uint8_t command);

/**
 * @brief Update register bank from hardware state
 */
void i2c_update_input_registers(void);

/**
 * @brief Apply register settings to hardware
 */
void i2c_apply_output_registers(void);

#endif // I2C_REGISTERS_H
