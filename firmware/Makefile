# Makefile for RP2040 Firmware Build
# PiCorePlayer Music Streamer - Peripheral Controller

.PHONY: all build upload clean monitor test help

# Default target
all: build

# Build firmware
build:
	@echo "Building firmware..."
	pio run

# Upload firmware via USB
upload:
	@echo "Uploading firmware..."
	pio run --target upload

# Upload via UF2 bootloader
upload-uf2:
	@echo "Building UF2..."
	pio run
	@echo ""
	@echo "Put RP2040 in bootloader mode (hold BOOTSEL, press RESET)"
	@echo "Then copy .pio/build/pico/firmware.uf2 to RPI-RP2 drive"
	@echo ""
	@echo "Or run: make copy-uf2 MOUNT=/media/RPI-RP2"

# Copy UF2 to mounted drive
copy-uf2:
	@if [ -z "$(MOUNT)" ]; then \
		echo "Error: MOUNT variable not set"; \
		echo "Usage: make copy-uf2 MOUNT=/path/to/RPI-RP2"; \
		exit 1; \
	fi
	@echo "Copying firmware to $(MOUNT)..."
	cp .pio/build/pico/firmware.uf2 $(MOUNT)/
	@echo "Done! RP2040 will automatically reboot."

# Clean build files
clean:
	@echo "Cleaning build files..."
	pio run --target clean
	rm -rf .pio

# Monitor serial output
monitor:
	@echo "Opening serial monitor (Ctrl+C to exit)..."
	pio device monitor

# List available serial ports
list-ports:
	@echo "Available serial ports:"
	pio device list

# Run tests (if implemented)
test:
	@echo "Running tests..."
	pio test

# Check code
check:
	@echo "Checking code..."
	pio check

# Show firmware info
info:
	@echo "Firmware Information:"
	@echo "  Project: PiCorePlayer Music Streamer - RP2040 Controller"
	@echo "  Board: Raspberry Pi Pico (RP2040)"
	@echo "  Framework: Arduino (Earlephilhower core)"
	@echo "  I2C Address: 0x42"
	@echo ""
	@echo "Build output: .pio/build/pico/"
	@echo "  firmware.elf - ELF binary"
	@echo "  firmware.uf2 - UF2 bootloader image"
	@echo "  firmware.hex - Intel HEX format"

# Show help
help:
	@echo "PiCorePlayer Music Streamer - RP2040 Firmware Build"
	@echo ""
	@echo "Available targets:"
	@echo "  make build        - Build firmware"
	@echo "  make upload       - Build and upload via USB"
	@echo "  make upload-uf2   - Build UF2 and show instructions"
	@echo "  make copy-uf2     - Copy UF2 to mounted RPI-RP2 drive"
	@echo "                      Usage: make copy-uf2 MOUNT=/media/RPI-RP2"
	@echo "  make clean        - Clean build files"
	@echo "  make monitor      - Open serial monitor"
	@echo "  make list-ports   - List available serial ports"
	@echo "  make test         - Run tests"
	@echo "  make check        - Check code quality"
	@echo "  make info         - Show firmware information"
	@echo "  make help         - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make              - Build firmware"
	@echo "  make upload       - Upload via USB"
	@echo "  make monitor      - View serial output"
	@echo ""
	@echo "For UF2 bootloader upload:"
	@echo "  1. make upload-uf2"
	@echo "  2. Hold BOOTSEL button on Pico"
	@echo "  3. Press and release RESET"
	@echo "  4. Release BOOTSEL"
	@echo "  5. make copy-uf2 MOUNT=/media/RPI-RP2"
