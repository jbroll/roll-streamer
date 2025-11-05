#!/usr/bin/env python3
"""
VU Meter Daemon for PiCorePlayer Music Streamer

This daemon reads audio levels from ALSA and updates the RP2040-controlled
VU meters with proper VU ballistics.

Based on the VU meter specifications from VUMeters.md:
- Integration time: 300ms time constant
- Rise time: 99% in 300ms
- Logarithmic response (dB scale)
"""

import sys
import time
import math
import signal
import logging
import argparse
import struct
from pathlib import Path
from typing import Optional

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


class VUMeter:
    """
    VU Meter ballistics implementation.

    Implements proper VU meter response with 300ms time constant.
    """

    def __init__(self, sample_rate: int = 44100, time_constant: float = 0.3):
        """
        Initialize VU meter.

        Args:
            sample_rate: Audio sample rate in Hz
            time_constant: VU time constant in seconds (default: 0.3 = 300ms)
        """
        self.sample_rate = sample_rate
        self.time_constant = time_constant

        # Calculate smoothing factor for exponential smoothing
        # alpha = 1 - exp(-1/(fs * tc))
        self.alpha = 1.0 - math.exp(-1.0 / (sample_rate * time_constant))
        self.vu_state = 0.0

    def process_samples(self, samples: list) -> float:
        """
        Process audio samples and return RMS level with VU ballistics.

        Args:
            samples: List of audio sample values

        Returns:
            RMS level (0.0 to 1.0)
        """
        if not samples:
            return 0.0

        # Calculate instantaneous RMS power
        power = sum(s * s for s in samples) / len(samples)

        # Apply VU ballistics (exponential smoothing)
        self.vu_state = self.alpha * power + (1.0 - self.alpha) * self.vu_state

        # Return RMS value
        return math.sqrt(max(0.0, self.vu_state))


class VUMeterDaemon:
    """
    VU Meter Daemon.

    Reads audio levels and updates physical VU meters.
    """

    def __init__(self, pipe_path: str = "/tmp/vu_meter_data",
                 update_rate: int = 50,
                 min_db: float = -20.0,
                 max_db: float = 3.0):
        """
        Initialize VU meter daemon.

        Args:
            pipe_path: Path to named pipe for audio data
            update_rate: Update rate in Hz (default: 50Hz)
            min_db: Minimum dB level for VU meter scale (default: -20 dB)
            max_db: Maximum dB level for VU meter scale (default: +3 dB)
        """
        self.pipe_path = Path(pipe_path)
        self.update_rate = update_rate
        self.min_db = min_db
        self.max_db = max_db
        self.running = False

        # Initialize VU meters
        self.vu_left = VUMeter()
        self.vu_right = VUMeter()

        # Initialize RP2040 controller
        self.controller = None

        # Reference level (0 VU = +4 dBu = 1.228V RMS)
        self.reference_voltage = 1.228

    def start(self):
        """Start the daemon."""
        logger.info("Starting VU Meter Daemon")

        # Connect to RP2040
        self.controller = RP2040Controller()
        if not self.controller.open():
            logger.error("Failed to connect to RP2040 controller")
            return False

        version = self.controller.get_firmware_version()
        logger.info(f"RP2040 firmware version: {version[0]}.{version[1]}.{version[2]}")

        # Enable VU meters
        self.controller.enable_vu_meters(True)
        self.controller.set_vu_mode(RP2040Controller.VU_MODE_NORMAL)

        self.running = True
        logger.info("VU Meter Daemon started")

        return True

    def stop(self):
        """Stop the daemon."""
        logger.info("Stopping VU Meter Daemon")
        self.running = False

        if self.controller:
            # Turn off VU meters
            self.controller.set_vu_meters(0, 0)
            self.controller.enable_vu_meters(False)
            self.controller.close()

        logger.info("VU Meter Daemon stopped")

    def scale_db_to_pwm(self, db_level: float) -> int:
        """
        Scale dB level to PWM value (0-255).

        Args:
            db_level: Level in dB

        Returns:
            PWM value (0-255)
        """
        # Clamp to meter range
        db_clamped = max(self.min_db, min(self.max_db, db_level))

        # Scale to 0-255
        pwm_percent = (db_clamped - self.min_db) / (self.max_db - self.min_db)
        pwm_value = int(pwm_percent * 255)

        return max(0, min(255, pwm_value))

    def rms_to_db(self, rms: float) -> float:
        """
        Convert RMS voltage to dB.

        Args:
            rms: RMS voltage (0.0 to 1.0 normalized)

        Returns:
            Level in dB
        """
        if rms < 1e-6:
            return self.min_db

        # Convert to dB relative to reference
        db = 20.0 * math.log10(rms / self.reference_voltage)
        return db

    def update_vu_meters(self, left_rms: float, right_rms: float):
        """
        Update physical VU meters.

        Args:
            left_rms: Left channel RMS level (0.0 to 1.0)
            right_rms: Right channel RMS level (0.0 to 1.0)
        """
        # Convert to dB
        left_db = self.rms_to_db(left_rms)
        right_db = self.rms_to_db(right_rms)

        # Scale to PWM
        left_pwm = self.scale_db_to_pwm(left_db)
        right_pwm = self.scale_db_to_pwm(right_db)

        # Update VU meters
        self.controller.set_vu_meters(left_pwm, right_pwm)

    def read_audio_pipe(self):
        """
        Read audio data from named pipe and update VU meters.

        This method expects the pipe to provide stereo float32 audio data.
        """
        logger.info(f"Opening audio pipe: {self.pipe_path}")

        # Wait for pipe to be created
        retry_count = 0
        while not self.pipe_path.exists() and retry_count < 10:
            logger.warning(f"Pipe {self.pipe_path} does not exist, waiting...")
            time.sleep(1)
            retry_count += 1

        if not self.pipe_path.exists():
            logger.error(f"Pipe {self.pipe_path} not found after {retry_count} retries")
            return

        try:
            with open(self.pipe_path, 'rb') as pipe:
                logger.info("Audio pipe opened, reading data...")

                buffer_size = 4096  # Read 4KB at a time
                sample_size = 8  # 2 channels * 4 bytes (float32)

                while self.running:
                    try:
                        # Read audio data
                        data = pipe.read(buffer_size)
                        if not data:
                            time.sleep(0.001)
                            continue

                        # Parse stereo float32 samples
                        num_samples = len(data) // sample_size
                        samples = struct.unpack(f'{num_samples * 2}f', data[:num_samples * sample_size])

                        # Separate left and right channels
                        left_samples = [samples[i] for i in range(0, len(samples), 2)]
                        right_samples = [samples[i] for i in range(1, len(samples), 2)]

                        # Process with VU ballistics
                        left_rms = self.vu_left.process_samples(left_samples)
                        right_rms = self.vu_right.process_samples(right_samples)

                        # Update VU meters
                        self.update_vu_meters(left_rms, right_rms)

                    except struct.error as e:
                        logger.error(f"Error parsing audio data: {e}")
                        continue

        except FileNotFoundError:
            logger.error(f"Pipe {self.pipe_path} not found")
        except BrokenPipeError:
            logger.warning("Pipe closed, retrying...")
            time.sleep(1)
        except Exception as e:
            logger.error(f"Error reading from pipe: {e}")

    def run_test_mode(self):
        """Run in test mode with generated sweep pattern."""
        logger.info("Running in test mode")

        try:
            # Sweep up
            for i in range(256):
                level = i
                self.controller.set_vu_meters(level, level)
                time.sleep(0.01)

            # Sweep down
            for i in range(255, -1, -1):
                level = i
                self.controller.set_vu_meters(level, level)
                time.sleep(0.01)

            # Turn off
            self.controller.set_vu_meters(0, 0)

        except KeyboardInterrupt:
            pass

    def run(self, test_mode: bool = False):
        """
        Run the daemon main loop.

        Args:
            test_mode: If True, run in test mode instead of reading audio
        """
        if not self.start():
            return 1

        try:
            if test_mode:
                self.run_test_mode()
            else:
                self.read_audio_pipe()

        except KeyboardInterrupt:
            logger.info("Received interrupt signal")
        except Exception as e:
            logger.error(f"Error in main loop: {e}", exc_info=True)
        finally:
            self.stop()

        return 0


def signal_handler(signum, frame):
    """Signal handler for graceful shutdown."""
    logger.info(f"Received signal {signum}")
    sys.exit(0)


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(description="VU Meter Daemon for PiCorePlayer")
    parser.add_argument(
        "--pipe",
        default="/tmp/vu_meter_data",
        help="Path to audio data named pipe (default: /tmp/vu_meter_data)"
    )
    parser.add_argument(
        "--rate",
        type=int,
        default=50,
        help="Update rate in Hz (default: 50)"
    )
    parser.add_argument(
        "--min-db",
        type=float,
        default=-20.0,
        help="Minimum dB level for VU scale (default: -20)"
    )
    parser.add_argument(
        "--max-db",
        type=float,
        default=3.0,
        help="Maximum dB level for VU scale (default: +3)"
    )
    parser.add_argument(
        "--test",
        action="store_true",
        help="Run in test mode (sweep pattern)"
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

    # Create and run daemon
    daemon = VUMeterDaemon(
        pipe_path=args.pipe,
        update_rate=args.rate,
        min_db=args.min_db,
        max_db=args.max_db
    )

    return daemon.run(test_mode=args.test)


if __name__ == "__main__":
    sys.exit(main())
