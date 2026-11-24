#!/bin/bash
OUT=$(printf "1\n2\nAli\nSara\n3\n3\n" | timeout 60 ./game | sed 's/\x1b\[[0-9;]*m//g')
check() { echo "$OUT" | grep -q "$1" || { echo "FAIL: $1"; exit 1; }; }
check "GAME 1 STARTED"
check "GAME 1 WINNER"
check "TOURNAMENT RESULT"
check "Server shutting down"
echo "ALL TESTS PASSED"
