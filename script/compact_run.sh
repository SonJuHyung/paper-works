#!/bin/bash 

trap 'handler' 2

RUNNING=1

# handler function
handler(){
    echo "SIG_INT signal accepted exit experiment!"
    RUNNING=0    
}

sleep 10s

THRESHOLD=1000
MODE=1
LEVEL_MIG=2
LEVEL_FREE=3

echo ${THRESHOLD} > /proc/son/pbstat_compact_mig_threshold 
echo ${MODE} > /proc/son/pbstat_compact_mode 
echo ${LEVEL_MIG} > /proc/son/pbstat_compact_mig_level
echo ${LEVEL_FREE} > /proc/son/pbstat_compact_free_level

while [ ${RUNNING} == 1 ];
do  
    echo 1 > /proc/sys/vm/compact_memory 
    echo "compacting ..."
    sleep 5s
done
