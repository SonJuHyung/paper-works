#!/bin/bash 

# compact monitor
MODE=2
# ppc
#MODE=1

# THRESHOLD=20
THRESHOLD=2000
LEVEL_MIG=1
LEVEL_FRE=3

echo ${MODE} > /proc/son/pbstat_compact_mode 
echo ${THRESHOLD} > /proc/son/pbstat_compact_mig_threshold  
# echo ${LEVEL_MIG} > /proc/son/pbstat_compact_mig_level
# echo ${LEVEL_FRE} > /proc/son/pbstat_compact_free_level

echo "before"
cat /proc/vmstat | grep migrate
cat /proc/son/pbstat_info_show_raw
echo ""
echo 1 > /proc/sys/vm/compact_memory 
echo ""
echo "after"
cat /proc/vmstat | grep migrate
cat /proc/son/pbstat_info_show_raw


