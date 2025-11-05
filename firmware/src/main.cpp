/**
 * @file main.cpp
 * @brief PiCorePlayer Music Streamer - RP2040 Peripheral Controller
 *
 * This firmware implements an I2C slave interface to control various peripherals
 * for a music streamer based on PiCorePlayer and RPi Zero 2.
 *
 * Features:
 * - I2C slave interface (address 0x42)
 * - 2x VU meter control via PWM (DRV8833)
 * - Panel backlight control via PWM (DRV8822)
 * - Tape counter motor control via PWM (DRV8833)
 * - 12x digital inputs with debouncing
 * - Rotary encoder with button
 *
 * Hardware: RP2040 (Raspberry Pi Pico)
 * Framework: Arduino (Earlephilhower core)
 */

#include <Arduino.h>
#include <Wire.h>
#include "pin_config.h"
#include "i2c_registers.h"

// ============================================================================
// Global Variables
// ============================================================================

// I2C Register Bank
I2CRegisterBank registers;

// Current register pointer for I2C reads
volatile uint8_t current_register = 0;

// Digital input state tracking
uint8_t input_state_low = 0xFF;
uint8_t input_state_high = 0x0F;
uint8_t input_prev_low = 0xFF;
uint8_t input_prev_high = 0x0F;
uint32_t input_last_change[NUM_DIGITAL_INPUTS] = {0};

// Encoder state tracking
volatile int16_t encoder_position = 0;
volatile int8_t encoder_delta = 0;
volatile uint8_t encoder_state = 0;
uint8_t encoder_button_state = ENC_BTN_RELEASED;
uint32_t encoder_button_press_time = 0;
uint32_t encoder_button_last_release = 0;

// PWM slices for each output
uint slice_vu_left, slice_vu_right;
uint slice_backlight;
uint slice_tape_motor;

// Status LED blink state
uint32_t led_last_blink = 0;
bool led_state = false;

// ============================================================================
// Function Prototypes
// ============================================================================

void setup_gpio(void);
void setup_pwm(void);
void setup_i2c(void);
void update_inputs(void);
void update_encoder(void);
void update_pwm_outputs(void);
void i2c_receive_handler(int byte_count);
void i2c_request_handler(void);
void set_motor_pwm(uint slice, uint8_t channel_a, uint8_t channel_b, uint8_t level);
void encoder_isr(void);

// ============================================================================
// Setup Function
// ============================================================================

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(1000);
    Serial.println("PiCorePlayer Music Streamer - RP2040 Controller");
    Serial.println("Firmware v1.0.0");

    // Initialize register bank
    i2c_registers_init();

    // Setup hardware
    setup_gpio();
    setup_pwm();
    setup_i2c();

    // Set status to ready
    registers.status = STATUS_READY;

    Serial.println("Initialization complete");
    Serial.println("I2C Address: 0x42");
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
    static uint32_t last_update = 0;
    uint32_t now = millis();

    // Update at 100Hz
    if (now - last_update >= 10) {
        last_update = now;

        // Update inputs
        update_inputs();

        // Update encoder
        update_encoder();

        // Update PWM outputs from register values
        update_pwm_outputs();

        // Blink status LED
        if (now - led_last_blink >= 500) {
            led_last_blink = now;
            led_state = !led_state;
            digitalWrite(PIN_STATUS_LED, led_state);
        }
    }

    // Small delay to prevent busy-waiting
    delay(1);
}

// ============================================================================
// GPIO Setup
// ============================================================================

void setup_gpio() {
    // Configure status LED
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);

    // Configure digital inputs with pull-ups
    for (int i = 0; i < NUM_DIGITAL_INPUTS; i++) {
        pinMode(DIGITAL_INPUT_PINS[i], INPUT_PULLUP);
    }

    // Configure encoder inputs with pull-ups
    pinMode(PIN_ENCODER_A, INPUT_PULLUP);
    pinMode(PIN_ENCODER_B, INPUT_PULLUP);
    pinMode(PIN_ENCODER_BTN, INPUT_PULLUP);

    // Attach interrupts for encoder
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_A), encoder_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_B), encoder_isr, CHANGE);

    Serial.println("GPIO configured");
}

// ============================================================================
// PWM Setup
// ============================================================================

void setup_pwm() {
    // Configure PWM pins as outputs
    pinMode(PIN_VU_LEFT_IN1, OUTPUT);
    pinMode(PIN_VU_LEFT_IN2, OUTPUT);
    pinMode(PIN_VU_RIGHT_IN1, OUTPUT);
    pinMode(PIN_VU_RIGHT_IN2, OUTPUT);
    pinMode(PIN_BACKLIGHT_IN1, OUTPUT);
    pinMode(PIN_BACKLIGHT_IN2, OUTPUT);
    pinMode(PIN_TAPE_MOTOR_IN1, OUTPUT);
    pinMode(PIN_TAPE_MOTOR_IN2, OUTPUT);

    // Set PWM frequency (1kHz)
    // RP2040 system clock is 125MHz
    // For 1kHz PWM with 8-bit resolution (0-255):
    // PWM frequency = clock / (wrap + 1) / divider
    // 1000 Hz = 125000000 / 256 / divider
    // divider = 125000000 / 256 / 1000 = 488.28
    analogWriteFreq(1000);  // 1kHz
    analogWriteRange(255);  // 8-bit resolution

    // Initialize all PWM outputs to 0
    analogWrite(PIN_VU_LEFT_IN1, 0);
    analogWrite(PIN_VU_LEFT_IN2, 0);
    analogWrite(PIN_VU_RIGHT_IN1, 0);
    analogWrite(PIN_VU_RIGHT_IN2, 0);
    analogWrite(PIN_BACKLIGHT_IN1, 0);
    analogWrite(PIN_BACKLIGHT_IN2, 0);
    analogWrite(PIN_TAPE_MOTOR_IN1, 0);
    analogWrite(PIN_TAPE_MOTOR_IN2, 0);

    Serial.println("PWM configured");
}

// ============================================================================
// I2C Setup
// ============================================================================

void setup_i2c() {
    // Initialize I2C1 as slave
    Wire1.setSDA(PIN_I2C_SDA);
    Wire1.setSCL(PIN_I2C_SCL);
    Wire1.begin(I2C_SLAVE_ADDRESS);

    // Register I2C event handlers
    Wire1.onReceive(i2c_receive_handler);
    Wire1.onRequest(i2c_request_handler);

    Serial.println("I2C slave configured at address 0x42");
}

// ============================================================================
// Update Functions
// ============================================================================

void update_inputs() {
    uint32_t now = millis();

    // Read digital inputs
    uint8_t new_state_low = 0xFF;
    uint8_t new_state_high = 0x0F;

    for (int i = 0; i < NUM_DIGITAL_INPUTS; i++) {
        bool state = digitalRead(DIGITAL_INPUT_PINS[i]);

        // Debounce
        if (state != ((i < 8 ? input_state_low : input_state_high) & (1 << (i % 8)))) {
            if (now - input_last_change[i] >= INPUT_DEBOUNCE_MS) {
                input_last_change[i] = now;
                if (i < 8) {
                    if (state) {
                        new_state_low |= (1 << i);
                    } else {
                        new_state_low &= ~(1 << i);
                    }
                } else {
                    if (state) {
                        new_state_high |= (1 << (i - 8));
                    } else {
                        new_state_high &= ~(1 << (i - 8));
                    }
                }
            } else {
                // Keep old state during debounce
                if (i < 8) {
                    if (input_state_low & (1 << i)) {
                        new_state_low |= (1 << i);
                    } else {
                        new_state_low &= ~(1 << i);
                    }
                } else {
                    if (input_state_high & (1 << (i - 8))) {
                        new_state_high |= (1 << (i - 8));
                    } else {
                        new_state_high &= ~(1 << (i - 8));
                    }
                }
            }
        } else {
            // State unchanged
            if (i < 8) {
                if (state) {
                    new_state_low |= (1 << i);
                } else {
                    new_state_low &= ~(1 << i);
                }
            } else {
                if (state) {
                    new_state_high |= (1 << (i - 8));
                } else {
                    new_state_high &= ~(1 << (i - 8));
                }
            }
        }
    }

    // Detect changes
    uint8_t changed_low = input_state_low ^ new_state_low;
    uint8_t changed_high = input_state_high ^ new_state_high;

    if (changed_low || changed_high) {
        registers.input_changed_low |= changed_low;
        registers.input_changed_high |= changed_high;
        registers.status |= STATUS_INPUT_CHANGED;
    }

    input_state_low = new_state_low;
    input_state_high = new_state_high;

    registers.input_status_low = input_state_low;
    registers.input_status_high = input_state_high;

    // Update encoder button
    bool btn_pressed = !digitalRead(PIN_ENCODER_BTN);
    uint32_t now_ms = millis();

    if (btn_pressed && encoder_button_state == ENC_BTN_RELEASED) {
        // Button just pressed
        encoder_button_state = ENC_BTN_PRESSED;
        encoder_button_press_time = now_ms;
        registers.status |= STATUS_BUTTON_PRESSED;

        // Check for double-click
        if (now_ms - encoder_button_last_release < 300) {
            encoder_button_state = ENC_BTN_DOUBLE_CLICK;
        }
    } else if (btn_pressed && encoder_button_state == ENC_BTN_PRESSED) {
        // Check if held long enough
        if (now_ms - encoder_button_press_time >= 1000) {
            encoder_button_state = ENC_BTN_HELD;
        }
    } else if (!btn_pressed && encoder_button_state != ENC_BTN_RELEASED) {
        // Button released
        encoder_button_last_release = now_ms;
        encoder_button_state = ENC_BTN_RELEASED;
        registers.status &= ~STATUS_BUTTON_PRESSED;
    }

    registers.encoder_button = encoder_button_state;
}

void update_encoder() {
    // Update encoder position in registers
    noInterrupts();
    int16_t pos = encoder_position;
    int8_t delta = encoder_delta;
    encoder_delta = 0;  // Clear delta after reading
    interrupts();

    registers.encoder_pos_low = pos & 0xFF;
    registers.encoder_pos_high = (pos >> 8) & 0xFF;
    registers.encoder_delta = delta;

    if (delta != 0) {
        registers.status |= STATUS_ENCODER_CHANGED;
    }
}

void update_pwm_outputs() {
    // Check if outputs are enabled
    bool vu_enabled = registers.control & CTRL_VU_ENABLE;
    bool backlight_enabled = registers.control & CTRL_BACKLIGHT_ENABLE;
    bool tape_enabled = registers.control & CTRL_TAPE_ENABLE;

    // Update VU meters
    if (vu_enabled && registers.vu_mode != VU_MODE_OFF) {
        analogWrite(PIN_VU_LEFT_IN1, registers.vu_left);
        analogWrite(PIN_VU_LEFT_IN2, 0);
        analogWrite(PIN_VU_RIGHT_IN1, registers.vu_right);
        analogWrite(PIN_VU_RIGHT_IN2, 0);
        registers.status |= STATUS_VU_ACTIVE;
    } else {
        analogWrite(PIN_VU_LEFT_IN1, 0);
        analogWrite(PIN_VU_LEFT_IN2, 0);
        analogWrite(PIN_VU_RIGHT_IN1, 0);
        analogWrite(PIN_VU_RIGHT_IN2, 0);
        registers.status &= ~STATUS_VU_ACTIVE;
    }

    // Update backlight
    if (backlight_enabled && registers.backlight_mode != BACKLIGHT_MODE_OFF) {
        analogWrite(PIN_BACKLIGHT_IN1, registers.backlight);
        analogWrite(PIN_BACKLIGHT_IN2, 0);
        registers.status |= STATUS_BACKLIGHT_ON;
    } else {
        analogWrite(PIN_BACKLIGHT_IN1, 0);
        analogWrite(PIN_BACKLIGHT_IN2, 0);
        registers.status &= ~STATUS_BACKLIGHT_ON;
    }

    // Update tape motor
    if (tape_enabled && registers.tape_mode != TAPE_MODE_OFF) {
        switch (registers.tape_direction) {
            case TAPE_DIR_FORWARD:
                analogWrite(PIN_TAPE_MOTOR_IN1, registers.tape_speed);
                analogWrite(PIN_TAPE_MOTOR_IN2, 0);
                registers.status |= STATUS_TAPE_RUNNING;
                break;
            case TAPE_DIR_REVERSE:
                analogWrite(PIN_TAPE_MOTOR_IN1, 0);
                analogWrite(PIN_TAPE_MOTOR_IN2, registers.tape_speed);
                registers.status |= STATUS_TAPE_RUNNING;
                break;
            case TAPE_DIR_BRAKE:
                analogWrite(PIN_TAPE_MOTOR_IN1, 255);
                analogWrite(PIN_TAPE_MOTOR_IN2, 255);
                registers.status &= ~STATUS_TAPE_RUNNING;
                break;
            case TAPE_DIR_STOP:
            default:
                analogWrite(PIN_TAPE_MOTOR_IN1, 0);
                analogWrite(PIN_TAPE_MOTOR_IN2, 0);
                registers.status &= ~STATUS_TAPE_RUNNING;
                break;
        }
    } else {
        analogWrite(PIN_TAPE_MOTOR_IN1, 0);
        analogWrite(PIN_TAPE_MOTOR_IN2, 0);
        registers.status &= ~STATUS_TAPE_RUNNING;
    }
}

// ============================================================================
// I2C Handlers
// ============================================================================

void i2c_receive_handler(int byte_count) {
    if (byte_count < 1) return;

    // First byte is register address
    current_register = Wire1.read();
    byte_count--;

    // If there are more bytes, this is a write operation
    while (byte_count > 0) {
        uint8_t value = Wire1.read();
        i2c_register_write(current_register, value);
        current_register++;
        byte_count--;
    }
}

void i2c_request_handler() {
    // Send requested register value
    uint8_t value = i2c_register_read(current_register);
    Wire1.write(value);
    current_register++;
}

// ============================================================================
// Encoder ISR
// ============================================================================

void encoder_isr() {
    // Read encoder pins
    uint8_t a = digitalRead(PIN_ENCODER_A);
    uint8_t b = digitalRead(PIN_ENCODER_B);

    // Combine into 2-bit state
    uint8_t new_state = (a << 1) | b;

    // State transition table for quadrature decoding
    // encoder_state is previous state, new_state is current state
    static const int8_t transition_table[4][4] = {
        { 0, -1,  1,  0},  // 00 -> 00, 01, 10, 11
        { 1,  0,  0, -1},  // 01 -> 00, 01, 10, 11
        {-1,  0,  0,  1},  // 10 -> 00, 01, 10, 11
        { 0,  1, -1,  0}   // 11 -> 00, 01, 10, 11
    };

    int8_t delta = transition_table[encoder_state][new_state];
    encoder_state = new_state;

    if (delta != 0) {
        encoder_position += delta;
        encoder_delta += delta;
    }
}

// ============================================================================
// Register Functions
// ============================================================================

void i2c_registers_init() {
    memset(&registers, 0, sizeof(registers));

    // Initialize device info
    registers.device_id = DEVICE_ID;
    registers.fw_ver_major = FIRMWARE_VERSION_MAJOR;
    registers.fw_ver_minor = FIRMWARE_VERSION_MINOR;
    registers.fw_ver_patch = FIRMWARE_VERSION_PATCH;

    // Initialize default values
    registers.control = CTRL_ENABLE;
    registers.status = STATUS_READY;
    registers.vu_mode = VU_MODE_NORMAL;
    registers.backlight = 128;
    registers.backlight_mode = BACKLIGHT_MODE_MANUAL;
    registers.tape_direction = TAPE_DIR_STOP;
    registers.tape_mode = TAPE_MODE_MANUAL;
    registers.input_status_low = 0xFF;
    registers.input_status_high = 0x0F;
    registers.config_vu_freq = 1;
    registers.config_debounce = INPUT_DEBOUNCE_MS;
}

uint8_t i2c_register_read(uint8_t reg_addr) {
    // Bounds check
    if (reg_addr >= sizeof(I2CRegisterBank)) {
        registers.error |= ERROR_INVALID_REG;
        return 0xFF;
    }

    // Read from register bank
    uint8_t *reg_ptr = (uint8_t*)&registers;
    uint8_t value = reg_ptr[reg_addr];

    // Auto-clear flags on read
    if (reg_addr == REG_INPUT_CHANGED_LOW) {
        registers.input_changed_low = 0;
        if (registers.input_changed_high == 0) {
            registers.status &= ~STATUS_INPUT_CHANGED;
        }
    } else if (reg_addr == REG_INPUT_CHANGED_HIGH) {
        registers.input_changed_high = 0;
        if (registers.input_changed_low == 0) {
            registers.status &= ~STATUS_INPUT_CHANGED;
        }
    } else if (reg_addr == REG_ENCODER_DELTA) {
        registers.status &= ~STATUS_ENCODER_CHANGED;
    }

    return value;
}

bool i2c_register_write(uint8_t reg_addr, uint8_t value) {
    // Check for read-only registers
    if (reg_addr <= 0x03) {
        registers.error |= ERROR_INVALID_REG;
        return false;
    }

    // Check for command register
    if (reg_addr == REG_COMMAND) {
        return i2c_execute_command(value);
    }

    // Bounds check
    if (reg_addr >= sizeof(I2CRegisterBank)) {
        registers.error |= ERROR_INVALID_REG;
        return false;
    }

    // Write to register bank
    uint8_t *reg_ptr = (uint8_t*)&registers;
    reg_ptr[reg_addr] = value;

    // Handle special control register operations
    if (reg_addr == REG_CONTROL) {
        if (value & CTRL_RESET_ENCODER) {
            noInterrupts();
            encoder_position = 0;
            encoder_delta = 0;
            interrupts();
            registers.encoder_pos_low = 0;
            registers.encoder_pos_high = 0;
            registers.encoder_delta = 0;
            // Clear the bit after action
            registers.control &= ~CTRL_RESET_ENCODER;
        }
        if (value & CTRL_CLEAR_INPUTS) {
            registers.input_changed_low = 0;
            registers.input_changed_high = 0;
            registers.status &= ~STATUS_INPUT_CHANGED;
            // Clear the bit after action
            registers.control &= ~CTRL_CLEAR_INPUTS;
        }
    }

    return true;
}

bool i2c_execute_command(uint8_t command) {
    switch (command) {
        case CMD_NOP:
            break;

        case CMD_RESET:
            // Soft reset
            i2c_registers_init();
            break;

        case CMD_TEST_VU_LEFT:
        case CMD_TEST_VU_RIGHT:
        case CMD_TEST_VU_BOTH:
        case CMD_TEST_BACKLIGHT:
        case CMD_TEST_TAPE_MOTOR:
        case CMD_TEST_ALL:
            // Test commands - implement test patterns
            // For now, just acknowledge
            break;

        default:
            registers.error |= ERROR_INVALID_CMD;
            return false;
    }

    return true;
}
