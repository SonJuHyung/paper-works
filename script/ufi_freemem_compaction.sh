#!/bin/bash 
COUNT=1
PRE_ISOLATED=0
COMPACT_SUCCESS=0
RUNNING=1

trap 'handler' 2

handler(){
    RUNNING=0
#    exit 0
}

#while :
while [ ${RUNNING} == 1 ]
do
    FRAG_CONTEXT=$(cat /sys/kernel/debug/extfrag/unusable_index | grep Normal)   
    FRAG=$(echo $FRAG_CONTEXT | awk '{ split($0,arr," "); print(arr[14]); }')
#    MEM_FREE=$(awk '$3=="kB"{if ($2>1024**2){$2=$2/1024**2;$3="GB";} else if ($2>1024){$2=$2/1024;$3="MB";}} 1' /proc/meminfo | column -t | grep MemFree)
    MEM_FREE=$(cat /proc/meminfo | grep "MemFree" | awk '{ split($0,parse1,":"); split(parse1[2],parse2,"kB"); print(parse2[1]); }')
    COMPACT_SUCCESS=$(cat /proc/vmstat | grep compact_success | awk '{ split($0,parse1,"compact_success"); print(parse1[2]) }')
    KCOMPACT_WAKEUP=$(cat /proc/vmstat | grep compact_daemon_wake | awk '{ split($0,parse1,"compact_daemon_wake"); print(parse1[2]) }')

    echo "${FRAG} ,${MEM_FREE}, ${COMPACT_SUCCESS}, ${KCOMPACT_WAKEUP}"
#    sleep "0.1s"
#    sleep "1s"
    sleep "0.5s"
done
