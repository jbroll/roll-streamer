#!/bin/sh
# Check status of roll-streamer services

echo "Roll-Streamer Service Status"
echo "=============================="

# Check VU Meter Daemon
if [ -f /var/run/vu-meter-daemon.pid ]; then
    PID=$(cat /var/run/vu-meter-daemon.pid)
    if ps -p $PID > /dev/null 2>&1; then
        echo "VU Meter Daemon: RUNNING (PID: $PID)"
    else
        echo "VU Meter Daemon: STOPPED (stale PID file)"
    fi
else
    echo "VU Meter Daemon: STOPPED"
fi

# Check Input Handler
if [ -f /var/run/input-handler.pid ]; then
    PID=$(cat /var/run/input-handler.pid)
    if ps -p $PID > /dev/null 2>&1; then
        echo "Input Handler: RUNNING (PID: $PID)"
    else
        echo "Input Handler: STOPPED (stale PID file)"
    fi
else
    echo "Input Handler: STOPPED"
fi

echo ""
echo "Recent log entries:"
echo "-------------------"
echo "VU Meter Daemon:"
tail -n 5 /tmp/vu-meter-daemon.log 2>/dev/null || echo "  No log file found"
echo ""
echo "Input Handler:"
tail -n 5 /tmp/input-handler.log 2>/dev/null || echo "  No log file found"
