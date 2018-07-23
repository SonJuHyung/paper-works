#!/bin/bash 

RUNNING=1
redis-cli config set latency-monitor-threshold 1

while [ ${RUNNING} == 1 ];
do  
    redis-cli latency latest 

    LAT_CONTEXT=$(redis-cli latency latest | grep -v "fast-command" | awk '/command/ {for(i=1; i<=3; i++) {getline; print}}' | sed -n '2,3p')
    CUR_LAT=$(echo ${LAT_CONTEXT} | cut -d " " -f 1)
    CUR_MAX=$(echo ${LAT_CONTEXT} | cut -d " " -f 2)
    echo "${CUR_LAT}, ${CUR_MAX}"
    sleep 1s
done
