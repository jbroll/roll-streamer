#!/usr/bin/env python3
"""
RP2040 Peripheral Controller Library
PiCorePlayer Music Streamer Extension

This library provides a Python interface to control the RP2040 peripheral
controller via I2C for VU meters, backlight, tape motor, and digital inputs.
"""

import logging
from typing import List, Tuple, Optional
from smbus2 import SMBus, i2c_msg
import time

# Configure logging
logger = logging.getLogger(__name__)


class RP2040Controller:
    """
    Python interface for RP2040 peripheral controller.

    Provides methods to control VU meters, backlight, tape motor,
    and read digital inputs and rotary encoder.
    """

    # I2C Configuration
    DEFAULT_BUS = 1
    DEFAULT_ADDRESS = 0x42

    # Device Information Registers
    REG_DEVICE_ID = 0x00
    REG_FIRMWARE_VER_MAJ = 0x01
    REG_FIRMWARE_VER_MIN = 0x02
    REG_FIRMWARE_VER_PATCH = 0x03

    # Control and Status Registers
    REG_CONTROL = 0x10
    REG_STATUS = 0x11
    REG_ERROR = 0x12

    # VU Meter Registers
    REG_VU_LEFT = 0x20
    REG_VU_RIGHT = 0x21
    REG_VU_MODE = 0x22

    # Backlight Registers
    REG_BACKLIGHT = 0x30
    REG_BACKLIGHT_MODE = 0x31

    # Tape Motor Registers
    REG_TAPE_SPEED = 0x40
    REG_TAPE_DIRECTION = 0x41
    REG_TAPE_MODE = 0x42

    # Digital Input Registers
    REG_INPUT_STATUS_LOW = 0x50
    REG_INPUT_STATUS_HIGH = 0x51
    REG_INPUT_CHANGED_LOW = 0x52
    REG_INPUT_CHANGED_HIGH = 0x53

    # Rotary Encoder Registers
    REG_ENCODER_POS_LOW = 0x60
    REG_ENCODER_POS_HIGH = 0x61
    REG_ENCODER_DELTA = 0x62
    REG_ENCODER_BUTTON = 0x63

    # Configuration Registers
    REG_CONFIG_VU_FREQ = 0x70
    REG_CONFIG_DEBOUNCE = 0x71
    REG_CONFIG_OPTIONS = 0x72

    # Command Register
    REG_COMMAND = 0xF0

    # Control Register Bits
    CTRL_ENABLE = 0x01
    CTRL_RESET_ENCODER = 0x02
    CTRL_CLEAR_INPUTS = 0x04
    CTRL_VU_ENABLE = 0x08
    CTRL_BACKLIGHT_ENABLE = 0x10
    CTRL_TAPE_ENABLE = 0x20
    CTRL_SAVE_CONFIG = 0x40
    CTRL_LOAD_CONFIG = 0x80

    # Status Register Bits
    STATUS_READY = 0x01
    STATUS_ERROR = 0x02
    STATUS_VU_ACTIVE = 0x04
    STATUS_BACKLIGHT_ON = 0x08
    STATUS_TAPE_RUNNING = 0x10
    STATUS_INPUT_CHANGED = 0x20
    STATUS_ENCODER_CHANGED = 0x40
    STATUS_BUTTON_PRESSED = 0x80

    # VU Modes
    VU_MODE_NORMAL = 0x00
    VU_MODE_PEAK_HOLD = 0x01
    VU_MODE_TEST = 0x02
    VU_MODE_OFF = 0xFF

    # Backlight Modes
    BACKLIGHT_MODE_MANUAL = 0x00
    BACKLIGHT_MODE_AUTO = 0x01
    BACKLIGHT_MODE_PULSE = 0x02
    BACKLIGHT_MODE_OFF = 0xFF

    # Tape Directions
    TAPE_DIR_STOP = 0x00
    TAPE_DIR_FORWARD = 0x01
    TAPE_DIR_REVERSE = 0x02
    TAPE_DIR_BRAKE = 0x03

    # Tape Modes
    TAPE_MODE_MANUAL = 0x00
    TAPE_MODE_AUTO = 0x01
    TAPE_MODE_OFF = 0xFF

    # Encoder Button States
    ENC_BTN_RELEASED = 0x00
    ENC_BTN_PRESSED = 0x01
    ENC_BTN_HELD = 0x02
    ENC_BTN_DOUBLE_CLICK = 0x03

    # Commands
    CMD_NOP = 0x00
    CMD_RESET = 0x01
    CMD_FACTORY_RESET = 0x02
    CMD_TEST_VU_LEFT = 0x11
    CMD_TEST_VU_RIGHT = 0x12
    CMD_TEST_VU_BOTH = 0x13
    CMD_TEST_BACKLIGHT = 0x20
    CMD_TEST_TAPE_MOTOR = 0x30
    CMD_TEST_ALL = 0xFF

    def __init__(self, bus: int = DEFAULT_BUS, address: int = DEFAULT_ADDRESS):
        """
        Initialize RP2040 controller interface.

        Args:
            bus: I2C bus number (default: 1)
            address: I2C slave address (default: 0x42)
        """
        self.bus_num = bus
        self.address = address
        self.bus = None
        self._last_encoder_pos = 0

    def open(self) -> bool:
        """
        Open I2C bus connection.

        Returns:
            True if successful, False otherwise
        """
        try:
            self.bus = SMBus(self.bus_num)
            logger.info(f"Opened I2C bus {self.bus_num}")

            # Verify device is present
            device_id = self.read_register(self.REG_DEVICE_ID)
            if device_id != 0x52:
                logger.error(f"Invalid device ID: 0x{device_id:02X} (expected 0x52)")
                return False

            logger.info("RP2040 controller detected")
            return True

        except Exception as e:
            logger.error(f"Failed to open I2C bus: {e}")
            return False

    def close(self):
        """Close I2C bus connection."""
        if self.bus:
            self.bus.close()
            logger.info("Closed I2C bus")

    def __enter__(self):
        """Context manager entry."""
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit."""
        self.close()

    def read_register(self, reg: int) -> int:
        """
        Read a single register.

        Args:
            reg: Register address

        Returns:
            Register value (0-255)
        """
        try:
            return self.bus.read_byte_data(self.address, reg)
        except Exception as e:
            logger.error(f"Failed to read register 0x{reg:02X}: {e}")
            raise

    def write_register(self, reg: int, value: int):
        """
        Write a single register.

        Args:
            reg: Register address
            value: Value to write (0-255)
        """
        try:
            self.bus.write_byte_data(self.address, reg, value & 0xFF)
        except Exception as e:
            logger.error(f"Failed to write register 0x{reg:02X}: {e}")
            raise

    def get_firmware_version(self) -> Tuple[int, int, int]:
        """
        Get firmware version.

        Returns:
            Tuple of (major, minor, patch)
        """
        major = self.read_register(self.REG_FIRMWARE_VER_MAJ)
        minor = self.read_register(self.REG_FIRMWARE_VER_MIN)
        patch = self.read_register(self.REG_FIRMWARE_VER_PATCH)
        return (major, minor, patch)

    def get_status(self) -> int:
        """
        Get status register.

        Returns:
            Status register value
        """
        return self.read_register(self.REG_STATUS)

    def get_error(self) -> int:
        """
        Get error register.

        Returns:
            Error register value
        """
        return self.read_register(self.REG_ERROR)

    def is_ready(self) -> bool:
        """Check if device is ready."""
        return (self.get_status() & self.STATUS_READY) != 0

    # ========================================================================
    # VU Meter Control
    # ========================================================================

    def set_vu_left(self, level: int):
        """
        Set left VU meter level.

        Args:
            level: VU level (0-255, 0=off, 255=full scale)
        """
        self.write_register(self.REG_VU_LEFT, level)

    def set_vu_right(self, level: int):
        """
        Set right VU meter level.

        Args:
            level: VU level (0-255, 0=off, 255=full scale)
        """
        self.write_register(self.REG_VU_RIGHT, level)

    def set_vu_meters(self, left: int, right: int):
        """
        Set both VU meter levels.

        Args:
            left: Left VU level (0-255)
            right: Right VU level (0-255)
        """
        self.set_vu_left(left)
        self.set_vu_right(right)

    def set_vu_mode(self, mode: int):
        """
        Set VU meter mode.

        Args:
            mode: VU mode (VU_MODE_NORMAL, VU_MODE_PEAK_HOLD, VU_MODE_TEST, VU_MODE_OFF)
        """
        self.write_register(self.REG_VU_MODE, mode)

    def enable_vu_meters(self, enable: bool = True):
        """
        Enable or disable VU meters.

        Args:
            enable: True to enable, False to disable
        """
        control = self.read_register(self.REG_CONTROL)
        if enable:
            control |= self.CTRL_VU_ENABLE
        else:
            control &= ~self.CTRL_VU_ENABLE
        self.write_register(self.REG_CONTROL, control)

    # ========================================================================
    # Backlight Control
    # ========================================================================

    def set_backlight(self, brightness: int):
        """
        Set backlight brightness.

        Args:
            brightness: Brightness level (0-255, 0=off, 255=full brightness)
        """
        self.write_register(self.REG_BACKLIGHT, brightness)

    def set_backlight_mode(self, mode: int):
        """
        Set backlight mode.

        Args:
            mode: Backlight mode (BACKLIGHT_MODE_MANUAL, BACKLIGHT_MODE_AUTO, etc.)
        """
        self.write_register(self.REG_BACKLIGHT_MODE, mode)

    def enable_backlight(self, enable: bool = True):
        """
        Enable or disable backlight.

        Args:
            enable: True to enable, False to disable
        """
        control = self.read_register(self.REG_CONTROL)
        if enable:
            control |= self.CTRL_BACKLIGHT_ENABLE
        else:
            control &= ~self.CTRL_BACKLIGHT_ENABLE
        self.write_register(self.REG_CONTROL, control)

    # ========================================================================
    # Tape Motor Control
    # ========================================================================

    def set_tape_motor(self, speed: int, direction: int = TAPE_DIR_FORWARD):
        """
        Set tape motor speed and direction.

        Args:
            speed: Motor speed (0-255)
            direction: Motor direction (TAPE_DIR_STOP, TAPE_DIR_FORWARD, TAPE_DIR_REVERSE)
        """
        self.write_register(self.REG_TAPE_SPEED, speed)
        self.write_register(self.REG_TAPE_DIRECTION, direction)

    def stop_tape_motor(self, brake: bool = False):
        """
        Stop tape motor.

        Args:
            brake: If True, use active brake; if False, coast to stop
        """
        self.write_register(self.REG_TAPE_SPEED, 0)
        direction = self.TAPE_DIR_BRAKE if brake else self.TAPE_DIR_STOP
        self.write_register(self.REG_TAPE_DIRECTION, direction)

    def enable_tape_motor(self, enable: bool = True):
        """
        Enable or disable tape motor.

        Args:
            enable: True to enable, False to disable
        """
        control = self.read_register(self.REG_CONTROL)
        if enable:
            control |= self.CTRL_TAPE_ENABLE
        else:
            control &= ~self.CTRL_TAPE_ENABLE
        self.write_register(self.REG_CONTROL, control)

    # ========================================================================
    # Digital Inputs
    # ========================================================================

    def get_digital_inputs(self) -> List[bool]:
        """
        Read all digital input states.

        Returns:
            List of 12 booleans (True = pressed/active, False = released/inactive)
        """
        status_low = self.read_register(self.REG_INPUT_STATUS_LOW)
        status_high = self.read_register(self.REG_INPUT_STATUS_HIGH)

        inputs = []
        # Inputs are active low, so invert the logic
        for i in range(8):
            inputs.append((status_low & (1 << i)) == 0)
        for i in range(4):
            inputs.append((status_high & (1 << i)) == 0)

        return inputs

    def get_input_changes(self) -> List[bool]:
        """
        Read input change flags.

        Returns:
            List of 12 booleans (True = input changed since last read)

        Note: Reading this register clears the change flags
        """
        changed_low = self.read_register(self.REG_INPUT_CHANGED_LOW)
        changed_high = self.read_register(self.REG_INPUT_CHANGED_HIGH)

        changes = []
        for i in range(8):
            changes.append((changed_low & (1 << i)) != 0)
        for i in range(4):
            changes.append((changed_high & (1 << i)) != 0)

        return changes

    def clear_input_changes(self):
        """Clear all input change flags."""
        control = self.read_register(self.REG_CONTROL)
        self.write_register(self.REG_CONTROL, control | self.CTRL_CLEAR_INPUTS)

    # ========================================================================
    # Rotary Encoder
    # ========================================================================

    def get_encoder_position(self) -> int:
        """
        Read encoder position.

        Returns:
            16-bit signed encoder position (-32768 to 32767)
        """
        low = self.read_register(self.REG_ENCODER_POS_LOW)
        high = self.read_register(self.REG_ENCODER_POS_HIGH)

        pos = (high << 8) | low
        # Convert to signed 16-bit
        if pos > 32767:
            pos -= 65536

        return pos

    def get_encoder_delta(self) -> int:
        """
        Read encoder delta since last read.

        Returns:
            8-bit signed delta (-128 to 127)

        Note: Reading this register clears the delta
        """
        delta = self.read_register(self.REG_ENCODER_DELTA)
        # Convert to signed 8-bit
        if delta > 127:
            delta -= 256

        return delta

    def get_encoder_button(self) -> int:
        """
        Read encoder button state.

        Returns:
            Button state (ENC_BTN_RELEASED, ENC_BTN_PRESSED, ENC_BTN_HELD, ENC_BTN_DOUBLE_CLICK)
        """
        return self.read_register(self.REG_ENCODER_BUTTON)

    def reset_encoder(self):
        """Reset encoder position to zero."""
        control = self.read_register(self.REG_CONTROL)
        self.write_register(self.REG_CONTROL, control | self.CTRL_RESET_ENCODER)

    # ========================================================================
    # Commands
    # ========================================================================

    def send_command(self, command: int):
        """
        Send a command to the controller.

        Args:
            command: Command code (CMD_*)
        """
        self.write_register(self.REG_COMMAND, command)

    def reset(self):
        """Soft reset the controller."""
        self.send_command(self.CMD_RESET)
        time.sleep(0.1)  # Wait for reset

    def test_vu_meters(self, which: str = "both"):
        """
        Test VU meters with sweep pattern.

        Args:
            which: "left", "right", or "both"
        """
        if which == "left":
            self.send_command(self.CMD_TEST_VU_LEFT)
        elif which == "right":
            self.send_command(self.CMD_TEST_VU_RIGHT)
        else:
            self.send_command(self.CMD_TEST_VU_BOTH)

    def test_backlight(self):
        """Test backlight with fade pattern."""
        self.send_command(self.CMD_TEST_BACKLIGHT)

    def test_all(self):
        """Test all outputs."""
        self.send_command(self.CMD_TEST_ALL)


# Example usage
if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)

    print("RP2040 Controller Test")
    print("-" * 40)

    try:
        with RP2040Controller() as controller:
            # Get firmware version
            version = controller.get_firmware_version()
            print(f"Firmware version: {version[0]}.{version[1]}.{version[2]}")

            # Enable VU meters and backlight
            controller.enable_vu_meters(True)
            controller.enable_backlight(True)

            # Set backlight to 50%
            controller.set_backlight(128)

            # Sweep VU meters
            print("\nSweeping VU meters...")
            for level in range(0, 256, 10):
                controller.set_vu_meters(level, level)
                time.sleep(0.05)

            for level in range(255, -1, -10):
                controller.set_vu_meters(level, level)
                time.sleep(0.05)

            # Read inputs
            print("\nReading digital inputs...")
            inputs = controller.get_digital_inputs()
            for i, state in enumerate(inputs, 1):
                print(f"  Input {i}: {'PRESSED' if state else 'released'}")

            # Read encoder
            print("\nEncoder position:", controller.get_encoder_position())
            print("Encoder button:", controller.get_encoder_button())

            print("\nTest complete!")

    except Exception as e:
        print(f"Error: {e}")
