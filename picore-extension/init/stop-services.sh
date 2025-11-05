#!/bin/sh
# Stop all roll-streamer services
# Can be added to /opt/shutdown.sh

echo "Stopping roll-streamer services..."

# Stop VU Meter Daemon
if [ -f /var/run/vu-meter-daemon.pid ]; then
    PID=$(cat /var/run/vu-meter-daemon.pid)
    if ps -p $PID > /dev/null 2>&1; then
        echo "Stopping VU Meter Daemon (PID: $PID)"
        kill $PID
        sleep 1
        # Force kill if still running
        if ps -p $PID > /dev/null 2>&1; then
            kill -9 $PID
        fi
    fi
    rm -f /var/run/vu-meter-daemon.pid
fi

# Stop Input Handler
if [ -f /var/run/input-handler.pid ]; then
    PID=$(cat /var/run/input-handler.pid)
    if ps -p $PID > /dev/null 2>&1; then
        echo "Stopping Input Handler (PID: $PID)"
        kill $PID
        sleep 1
        # Force kill if still running
        if ps -p $PID > /dev/null 2>&1; then
            kill -9 $PID
        fi
    fi
    rm -f /var/run/input-handler.pid
fi

echo "All roll-streamer services stopped"
