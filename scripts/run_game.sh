#!/bin/bash
mkdir -p logs
TS=$(date +%Y%m%d_%H%M%S)
LOG="logs/game_$TS.log"
INPUT=${1:-"1\n2\nAli\nSara\n3\n3\n"}

printf "$INPUT" | ./game > "$LOG" 2>&1 &
GAME_PID=$!
echo "Game started, PID=$GAME_PID, log=$LOG"

bash scripts/monitor.sh $GAME_PID &
MON_PID=$!

wait $GAME_PID
kill $MON_PID 2>/dev/null
echo "Game finished."
bash scripts/collect_logs.sh "$LOG"
