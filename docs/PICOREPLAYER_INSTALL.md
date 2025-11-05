# PiCorePlayer Installation Guide

Complete installation guide specifically for PiCorePlayer (Tiny Core Linux).

## Important: PiCorePlayer Differences

PiCorePlayer is based on **Tiny Core Linux**, not a standard Linux distribution:
- ❌ **NO systemd** - Uses traditional init scripts
- ❌ **NO systemctl** - Use custom startup scripts
- ✅ **Uses /opt/bootlocal.sh** for startup
- ✅ **Uses filetool.sh -b** to persist changes
- ✅ **Runs from RAM** - changes need to be saved to persist

## Prerequisites

### Hardware Required
- Raspberry Pi Zero 2 W with PiCorePlayer installed
- RP2040 (Raspberry Pi Pico) with firmware uploaded
- All peripherals wired according to [Wiring Diagram](hardware/Wiring_Diagram.md)

### Before You Begin
1. PiCorePlayer should be installed and running
2. RP2040 firmware should be uploaded (see firmware/README or main README)
3. I2C should be wired between RPi and RP2040
4. You should be able to access PiCorePlayer via SSH or web interface

## Step 1: Enable I2C on PiCorePlayer

### Via Web Interface
1. Navigate to: http://picoreplayer.local
2. Go to: **Main Page → Tweaks**
3. Set **I2C**: **Yes**
4. Click **Save**
5. Reboot PiCorePlayer

### Via Command Line
```bash
ssh tc@picoreplayer.local
# Password: piCore

# Edit bootlocal.sh
sudo vi /opt/bootlocal.sh

# Add this line:
modprobe i2c-dev

# Save changes
sudo filetool.sh -b

# Reboot
sudo reboot
```

### Verify I2C
```bash
ssh tc@picoreplayer.local

# Check I2C devices
ls /dev/i2c-*
# Should see: /dev/i2c-1

# Scan for RP2040 (should show 0x42)
i2cdetect -y 1
```

## Step 2: Install Required Packages

PiCorePlayer uses the Tiny Core package manager (`tce-load`):

```bash
ssh tc@picoreplayer.local

# Load required packages
tce-load -wi python3.6.tcz
tce-load -wi i2c-tools.tcz
tce-load -wi python3.6-pip.tcz

# Install Python library
sudo pip3 install smbus2

# Make persistent
sudo filetool.sh -b
```

## Step 3: Install Roll-Streamer Files

### Transfer Files to PiCorePlayer

From your development machine:

```bash
# Copy entire extension directory
scp -r picore-extension tc@picoreplayer.local:/tmp/

# Or copy individual components:
scp -r picore-extension/src tc@picoreplayer.local:/tmp/
scp -r picore-extension/scripts tc@picoreplayer.local:/tmp/
scp -r picore-extension/init tc@picoreplayer.local:/tmp/
```

### Install on PiCorePlayer

```bash
ssh tc@picoreplayer.local

# Create installation directory
sudo mkdir -p /opt/roll-streamer

# Copy files
sudo cp -r /tmp/picore-extension/src /opt/roll-streamer/
sudo cp -r /tmp/picore-extension/scripts /opt/roll-streamer/
sudo cp -r /tmp/picore-extension/init /opt/roll-streamer/

# Set executable permissions
sudo chmod +x /opt/roll-streamer/scripts/*.py
sudo chmod +x /opt/roll-streamer/init/*.sh

# Add to persistence list
echo "/opt/roll-streamer" | sudo tee -a /opt/.filetool.lst

# Save changes
sudo filetool.sh -b
```

## Step 4: Configure Startup Scripts

PiCorePlayer uses `/opt/bootlocal.sh` for startup commands:

```bash
ssh tc@picoreplayer.local

# Backup bootlocal.sh
sudo cp /opt/bootlocal.sh /opt/bootlocal.sh.backup

# Edit bootlocal.sh
sudo vi /opt/bootlocal.sh
```

Add these lines at the end of `/opt/bootlocal.sh`:

```bash
# Roll-Streamer Services
echo "Starting Roll-Streamer services..."

# Load I2C module (if not already loaded)
modprobe i2c-dev

# Wait for I2C to be ready
sleep 2

# Start VU Meter Daemon
/opt/roll-streamer/init/start-vu-meter.sh &

# Start Input Handler
/opt/roll-streamer/init/start-input-handler.sh &

echo "Roll-Streamer services started"
```

Save and exit (`:wq` in vi).

### Make Bootlocal Persistent

```bash
# Save changes
sudo filetool.sh -b
```

## Step 5: Test Installation

### Test I2C Communication

```bash
# Should show device at 0x42
i2cdetect -y 1

# Read device ID (should return 0x52)
i2cget -y 1 0x42 0x00

# If you see 0x52, communication is working!
```

### Test Python Library

```bash
python3 /opt/roll-streamer/src/rp2040_controller.py
```

Expected output:
```
RP2040 Controller Test
----------------------------------------
Firmware version: 1.0.0
Sweeping VU meters...
...
```

### Test VU Meters

```bash
# Run test pattern
python3 /opt/roll-streamer/scripts/vu_meter_daemon.py --test
```

You should see the VU meters sweep from 0 to 100% and back.

## Step 6: Start Services Manually (First Time)

```bash
# Start VU meter daemon
/opt/roll-streamer/init/start-vu-meter.sh

# Start input handler
/opt/roll-streamer/init/start-input-handler.sh

# Check status
/opt/roll-streamer/init/check-services.sh
```

Expected output:
```
Roll-Streamer Service Status
==============================
VU Meter Daemon: RUNNING (PID: 1234)
Input Handler: RUNNING (PID: 1235)
```

## Step 7: Verify Automatic Startup

```bash
# Reboot PiCorePlayer
sudo reboot

# After reboot, check services
/opt/roll-streamer/init/check-services.sh

# View logs
tail -f /tmp/vu-meter-daemon.log
tail -f /tmp/input-handler.log
```

## Service Management

### Check Service Status

```bash
/opt/roll-streamer/init/check-services.sh
```

### Stop Services

```bash
/opt/roll-streamer/init/stop-services.sh
```

### Restart Services

```bash
/opt/roll-streamer/init/stop-services.sh
/opt/roll-streamer/init/start-vu-meter.sh
/opt/roll-streamer/init/start-input-handler.sh
```

### View Logs

```bash
# VU Meter Daemon log
tail -f /tmp/vu-meter-daemon.log

# Input Handler log
tail -f /tmp/input-handler.log

# View last 50 lines
tail -n 50 /tmp/vu-meter-daemon.log
```

## Configuration

### Customize Input Mappings

Edit `/opt/roll-streamer/scripts/input_handler.py`:

```bash
sudo vi /opt/roll-streamer/scripts/input_handler.py

# Find DEFAULT_INPUT_MAP and modify button functions
```

After editing:
```bash
sudo filetool.sh -b  # Save changes
/opt/roll-streamer/init/stop-services.sh
/opt/roll-streamer/init/start-input-handler.sh
```

### Adjust VU Meter Calibration

Edit `/opt/roll-streamer/scripts/vu_meter_daemon.py`:

```bash
sudo vi /opt/roll-streamer/scripts/vu_meter_daemon.py

# Modify these parameters:
# - min_db: Minimum dB level (-20.0)
# - max_db: Maximum dB level (3.0)
# - update_rate: Update frequency in Hz (50)
```

After editing:
```bash
sudo filetool.sh -b  # Save changes
/opt/roll-streamer/init/stop-services.sh
/opt/roll-streamer/init/start-vu-meter.sh
```

## Persistence in PiCorePlayer

**IMPORTANT:** PiCorePlayer runs entirely from RAM!

Changes must be saved using:
```bash
sudo filetool.sh -b
```

Files/directories that should persist must be listed in `/opt/.filetool.lst`:
```bash
# View persistence list
cat /opt/.filetool.lst

# Add directory to persistence
echo "/opt/roll-streamer" | sudo tee -a /opt/.filetool.lst

# Save
sudo filetool.sh -b
```

## Troubleshooting

### Services Not Starting After Reboot

**Check bootlocal.sh:**
```bash
cat /opt/bootlocal.sh
# Should include roll-streamer startup commands
```

**Check if file is persistent:**
```bash
grep bootlocal /opt/.filetool.lst
# Should show: opt/bootlocal.sh
```

**Manually save bootlocal.sh:**
```bash
sudo filetool.sh -b
```

### I2C Device Not Found

```bash
# Check I2C module is loaded
lsmod | grep i2c

# Load manually
sudo modprobe i2c-dev

# Scan for devices
i2cdetect -y 1

# Check wiring if device not at 0x42
```

### Python Import Errors

```bash
# Check if smbus2 is installed
python3 -c "import smbus2; print('OK')"

# Reinstall if needed
sudo pip3 install smbus2

# Make persistent
sudo filetool.sh -b
```

### Services Start But Don't Work

**Check logs:**
```bash
tail -f /tmp/vu-meter-daemon.log
tail -f /tmp/input-handler.log
```

**Check PID files:**
```bash
ls -l /var/run/vu-meter-daemon.pid
ls -l /var/run/input-handler.pid
```

**Verify processes are running:**
```bash
ps aux | grep python3
```

### VU Meters Not Responding

**Test I2C:**
```bash
i2cget -y 1 0x42 0x00  # Should return 0x52
```

**Manually enable and test:**
```bash
i2cset -y 1 0x42 0x10 0x09  # Enable VU meters
i2cset -y 1 0x42 0x20 255   # Left VU to max
i2cset -y 1 0x42 0x21 255   # Right VU to max
```

**Check daemon is running:**
```bash
/opt/roll-streamer/init/check-services.sh
```

## PiCorePlayer-Specific Notes

### File Persistence
Only files/directories in `/opt/.filetool.lst` persist across reboots.

**Already persistent in PiCorePlayer:**
- `/opt/bootlocal.sh`
- `/opt/shutdown.sh`
- `/opt/.filetool.lst`
- `/home/tc`

**Need to add for roll-streamer:**
- `/opt/roll-streamer`

### System Logs
- Logs go to `/tmp/` which is in RAM
- Logs are lost on reboot
- Consider adding log rotation if needed

### Resource Limitations
PiCorePlayer is designed to be minimal:
- Limited RAM (~256MB usable)
- Limited CPU on RPi Zero 2
- Keep daemons lightweight
- Monitor resource usage: `top` or `htop`

### Package Management
- Use `tce-load -wi` to install packages
- Packages need to be re-downloaded if not backed up
- Consider using `tce-load -wil package.tcz.list` for batch install

## Quick Command Reference

```bash
# Service Management
/opt/roll-streamer/init/check-services.sh   # Check status
/opt/roll-streamer/init/start-vu-meter.sh   # Start VU daemon
/opt/roll-streamer/init/start-input-handler.sh  # Start input handler
/opt/roll-streamer/init/stop-services.sh    # Stop all services

# View Logs
tail -f /tmp/vu-meter-daemon.log   # VU meter log
tail -f /tmp/input-handler.log     # Input handler log

# I2C Testing
i2cdetect -y 1                     # Scan I2C bus
i2cget -y 1 0x42 0x00              # Read device ID

# Persistence
sudo filetool.sh -b                # Save changes

# Reboot
sudo reboot                        # Restart system
```

## Additional Resources

- [PiCorePlayer Documentation](https://docs.picoreplayer.org/)
- [Tiny Core Linux Wiki](http://wiki.tinycorelinux.net/)
- [Main README](../README.md)
- [Hardware Wiring Guide](hardware/Wiring_Diagram.md)
- [I2C Register Map](hardware/I2C_Register_Map.md)
- [Quick Reference](../QUICK_REFERENCE.md)

## Next Steps

1. Test all peripherals individually
2. Configure button mappings
3. Calibrate VU meters
4. Test with actual audio playback
5. Verify automatic startup on boot
6. Enjoy your music streamer! 🎵

---

**PiCorePlayer Installation Complete!**
