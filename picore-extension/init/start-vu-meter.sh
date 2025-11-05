#!/bin/sh
# VU Meter Daemon Startup Script for PiCorePlayer
# Add this to /opt/bootlocal.sh

LOG_FILE=/tmp/vu-meter-daemon.log
PID_FILE=/var/run/vu-meter-daemon.pid

echo "Starting VU Meter Daemon..." | tee -a $LOG_FILE

# Check if already running
if [ -f $PID_FILE ]; then
    PID=$(cat $PID_FILE)
    if ps -p $PID > /dev/null 2>&1; then
        echo "VU Meter Daemon already running (PID: $PID)" | tee -a $LOG_FILE
        exit 0
    fi
fi

# Start daemon in background
/usr/local/bin/python3 /opt/roll-streamer/scripts/vu_meter_daemon.py \
    --pipe /tmp/vu_meter_data \
    >> $LOG_FILE 2>&1 &

# Save PID
echo $! > $PID_FILE

echo "VU Meter Daemon started (PID: $(cat $PID_FILE))" | tee -a $LOG_FILE
