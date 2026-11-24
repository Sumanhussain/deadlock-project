#!/bin/bash
LOG=$1
CLEAN=$(sed 's/\x1b\[[0-9;]*m//g' "$LOG")
echo "===== LOG SUMMARY ====="
echo "Deadlocks detected : $(echo "$CLEAN" | grep -c 'DEADLOCK')"
echo "Recoveries         : $(echo "$CLEAN" | grep -c 'recovered successfully')"
echo "Resource acquires  : $(echo "$CLEAN" | grep -c 'acquired resources')"
echo "$CLEAN" | grep 'WINNER'
echo "System load        : $(uptime)"
echo "Free memory        : $(free -h | awk 'NR==2{print $4}')"
