#!/bin/sh
# Input Handler Startup Script for PiCorePlayer
# Add this to /opt/bootlocal.sh

LOG_FILE=/tmp/input-handler.log
PID_FILE=/var/run/input-handler.pid

echo "Starting Input Handler..." | tee -a $LOG_FILE

# Check if already running
if [ -f $PID_FILE ]; then
    PID=$(cat $PID_FILE)
    if ps -p $PID > /dev/null 2>&1; then
        echo "Input Handler already running (PID: $PID)" | tee -a $LOG_FILE
        exit 0
    fi
fi

# Start handler in background
/usr/local/bin/python3 /opt/roll-streamer/scripts/input_handler.py \
    --poll-rate 20 \
    >> $LOG_FILE 2>&1 &

# Save PID
echo $! > $PID_FILE

echo "Input Handler started (PID: $(cat $PID_FILE))" | tee -a $LOG_FILE
