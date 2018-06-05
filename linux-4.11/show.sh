#!/bin/bash 

# compact monitor
MODE=2
# ppc
#MODE=1

#THRESHOLD=20
THRESHOLD=100

echo ${MODE} > /proc/son/pbstat_compact_mode 
echo ${THRESHOLD} > /proc/son/pbstat_compact_mig_threshold 

echo "before"
cat /proc/vmstat | grep migrate
cat /proc/son/pbstat_info_show_raw
echo ""
echo 1 > /proc/sys/vm/compact_memory 
echo ""
echo "after"
cat /proc/vmstat | grep migrate
cat /proc/son/pbstat_info_show_raw


