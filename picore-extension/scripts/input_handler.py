#!/usr/bin/env python3
"""
Input Handler for PiCorePlayer Music Streamer

This script monitors digital inputs and rotary encoder from the RP2040
controller and performs corresponding actions in PiCorePlayer.

Features:
- Monitor 12 digital inputs (buttons/switches)
- Monitor rotary encoder for volume control
- Monitor encoder button
- Execute commands via pcp (PiCorePlayer command-line tools)
"""

import sys
import time
import signal
import logging
import argparse
import subprocess
from pathlib import Path
from typing import Optional, Callable, Dict

try:
    from rp2040_controller import RP2040Controller
except ImportError:
    sys.path.insert(0, str(Path(__file__).parent.parent / "src"))
    from rp2040_controller import RP2040Controller

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class SqueezeliteControl:
    """
    Control interface for Squeezelite/LMS (Logitech Media Server).

    Uses pcp (PiCorePlayer) command-line tools to control playback.
    """

    @staticmethod
    def execute_command(command: str) -> bool:
        """
        Execute a shell command.

        Args:
            command: Command to execute

        Returns:
            True if successful, False otherwise
        """
        try:
            result = subprocess.run(
                command,
                shell=True,
                capture_output=True,
                text=True,
                timeout=5
            )
            if result.returncode != 0:
                logger.error(f"Command failed: {command}\n{result.stderr}")
                return False
            return True
        except subprocess.TimeoutExpired:
            logger.error(f"Command timed out: {command}")
            return False
        except Exception as e:
            logger.error(f"Error executing command: {e}")
            return False

    @staticmethod
    def play_pause():
        """Toggle play/pause."""
        logger.info("Play/Pause")
        # Use pcp command or direct squeezelite control
        SqueezeliteControl.execute_command("pcp play_pause || echo 'pause' | nc localhost 3483")

    @staticmethod
    def play():
        """Start playback."""
        logger.info("Play")
        SqueezeliteControl.execute_command("pcp play || echo 'play' | nc localhost 3483")

    @staticmethod
    def pause():
        """Pause playback."""
        logger.info("Pause")
        SqueezeliteControl.execute_command("pcp pause || echo 'pause' | nc localhost 3483")

    @staticmethod
    def stop():
        """Stop playback."""
        logger.info("Stop")
        SqueezeliteControl.execute_command("pcp stop || echo 'stop' | nc localhost 3483")

    @staticmethod
    def next_track():
        """Skip to next track."""
        logger.info("Next Track")
        SqueezeliteControl.execute_command("pcp next || echo 'button jump_fwd' | nc localhost 3483")

    @staticmethod
    def previous_track():
        """Go to previous track."""
        logger.info("Previous Track")
        SqueezeliteControl.execute_command("pcp previous || echo 'button jump_rew' | nc localhost 3483")

    @staticmethod
    def volume_up(amount: int = 5):
        """
        Increase volume.

        Args:
            amount: Volume increase amount (0-100)
        """
        logger.info(f"Volume Up (+{amount})")
        SqueezeliteControl.execute_command(f"amixer sset 'Digital' {amount}%+")

    @staticmethod
    def volume_down(amount: int = 5):
        """
        Decrease volume.

        Args:
            amount: Volume decrease amount (0-100)
        """
        logger.info(f"Volume Down (-{amount})")
        SqueezeliteControl.execute_command(f"amixer sset 'Digital' {amount}%-")

    @staticmethod
    def set_volume(level: int):
        """
        Set absolute volume level.

        Args:
            level: Volume level (0-100)
        """
        logger.info(f"Set Volume: {level}")
        SqueezeliteControl.execute_command(f"amixer sset 'Digital' {level}%")

    @staticmethod
    def mute_toggle():
        """Toggle mute."""
        logger.info("Mute Toggle")
        SqueezeliteControl.execute_command("amixer sset 'Digital' toggle")


class InputHandler:
    """
    Input handler for digital inputs and rotary encoder.
    """

    # Default input mappings
    DEFAULT_INPUT_MAP = {
        1: ("Play/Pause", SqueezeliteControl.play_pause),
        2: ("Stop", SqueezeliteControl.stop),
        3: ("Next Track", SqueezeliteControl.next_track),
        4: ("Previous Track", SqueezeliteControl.previous_track),
        5: ("Volume Up", lambda: SqueezeliteControl.volume_up(5)),
        6: ("Volume Down", lambda: SqueezeliteControl.volume_down(5)),
        7: ("Mute", SqueezeliteControl.mute_toggle),
        8: ("Input 8", lambda: logger.info("Input 8 pressed")),
        9: ("Input 9", lambda: logger.info("Input 9 pressed")),
        10: ("Input 10", lambda: logger.info("Input 10 pressed")),
        11: ("Input 11", lambda: logger.info("Input 11 pressed")),
        12: ("Input 12", lambda: logger.info("Input 12 pressed")),
    }

    def __init__(self, controller: RP2040Controller):
        """
        Initialize input handler.

        Args:
            controller: RP2040Controller instance
        """
        self.controller = controller
        self.input_map = self.DEFAULT_INPUT_MAP.copy()
        self.running = False

        # Encoder state
        self.last_encoder_pos = 0
        self.encoder_volume_mode = True  # True = volume control, False = track selection

    def set_input_action(self, input_num: int, name: str, action: Callable):
        """
        Set action for a specific input.

        Args:
            input_num: Input number (1-12)
            name: Descriptive name for the action
            action: Callable to execute when input is pressed
        """
        if 1 <= input_num <= 12:
            self.input_map[input_num] = (name, action)
            logger.info(f"Input {input_num} mapped to: {name}")

    def handle_digital_inputs(self):
        """Check and handle digital input changes."""
        # Get inputs that have changed
        changes = self.controller.get_input_changes()

        # Get current input states
        inputs = self.controller.get_digital_inputs()

        # Process changed inputs
        for i, changed in enumerate(changes, 1):
            if changed and inputs[i - 1]:  # Input is pressed and changed
                if i in self.input_map:
                    name, action = self.input_map[i]
                    logger.info(f"Input {i} ({name}) pressed")
                    try:
                        action()
                    except Exception as e:
                        logger.error(f"Error executing action for input {i}: {e}")

    def handle_encoder(self):
        """Handle rotary encoder rotation and button."""
        # Get encoder delta
        delta = self.controller.get_encoder_delta()

        if delta != 0:
            if self.encoder_volume_mode:
                # Volume control mode
                if delta > 0:
                    SqueezeliteControl.volume_up(abs(delta))
                else:
                    SqueezeliteControl.volume_down(abs(delta))
            else:
                # Track selection mode (not implemented)
                logger.info(f"Encoder delta: {delta}")

        # Handle encoder button
        button = self.controller.get_encoder_button()

        if button == RP2040Controller.ENC_BTN_PRESSED:
            logger.info("Encoder button pressed")
            SqueezeliteControl.play_pause()
        elif button == RP2040Controller.ENC_BTN_HELD:
            logger.info("Encoder button held")
            # Toggle encoder mode
            self.encoder_volume_mode = not self.encoder_volume_mode
            mode_str = "Volume" if self.encoder_volume_mode else "Track Selection"
            logger.info(f"Encoder mode: {mode_str}")
        elif button == RP2040Controller.ENC_BTN_DOUBLE_CLICK:
            logger.info("Encoder button double-clicked")
            SqueezeliteControl.mute_toggle()

    def run(self, poll_rate: int = 20):
        """
        Run the input handler main loop.

        Args:
            poll_rate: Polling rate in Hz (default: 20Hz)
        """
        logger.info("Starting Input Handler")
        self.running = True

        poll_interval = 1.0 / poll_rate

        try:
            while self.running:
                start_time = time.time()

                # Handle digital inputs
                self.handle_digital_inputs()

                # Handle encoder
                self.handle_encoder()

                # Sleep for remaining time to maintain poll rate
                elapsed = time.time() - start_time
                sleep_time = max(0, poll_interval - elapsed)
                if sleep_time > 0:
                    time.sleep(sleep_time)

        except KeyboardInterrupt:
            logger.info("Received interrupt signal")
        finally:
            logger.info("Input Handler stopped")

    def stop(self):
        """Stop the input handler."""
        self.running = False


def signal_handler(signum, frame):
    """Signal handler for graceful shutdown."""
    logger.info(f"Received signal {signum}")
    sys.exit(0)


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Input Handler for PiCorePlayer Music Streamer"
    )
    parser.add_argument(
        "--poll-rate",
        type=int,
        default=20,
        help="Input polling rate in Hz (default: 20)"
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Enable verbose logging"
    )

    args = parser.parse_args()

    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)

    # Register signal handlers
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    # Connect to RP2040
    logger.info("Connecting to RP2040 controller...")
    controller = RP2040Controller()

    if not controller.open():
        logger.error("Failed to connect to RP2040 controller")
        return 1

    version = controller.get_firmware_version()
    logger.info(f"RP2040 firmware version: {version[0]}.{version[1]}.{version[2]}")

    # Create and run input handler
    handler = InputHandler(controller)

    try:
        handler.run(poll_rate=args.poll_rate)
    except Exception as e:
        logger.error(f"Error in input handler: {e}", exc_info=True)
        return 1
    finally:
        controller.close()

    return 0


if __name__ == "__main__":
    sys.exit(main())
