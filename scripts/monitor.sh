#!/bin/bash
PID=$1
MLOG="logs/monitor_$(date +%Y%m%d_%H%M%S).log"
echo "time,pid,threads,cpu%,mem%" > "$MLOG"
while kill -0 "$PID" 2>/dev/null; do
    CHILD=$(pgrep -P "$PID" | head -1)
    TARGET=${CHILD:-$PID}
    THREADS=$(ps -o nlwp= -p "$TARGET" 2>/dev/null | tr -d ' ')
    STATS=$(ps -o %cpu=,%mem= -p "$TARGET" 2>/dev/null)
    echo "$(date +%T),$TARGET,${THREADS:-0},$STATS" >> "$MLOG"
    sleep 1
done
