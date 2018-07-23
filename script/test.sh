#!/bin/bash 

PID=$(pgrep redis)

echo ${PID} 

TEST=$(cat /proc/son/pbstat_compact_mode)
echo ${TEST}
