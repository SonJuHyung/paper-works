#!/bin/bash 

DIR_EXPR=$(pwd)/.. 
DIR_DATA=${DIR_EXPR}/data/pbusage
PROC_SCAN_ENABLE=/proc/son/scan_pbstate_enable 
PROC_DEBUG_ENABLE=/proc/son/debug
VERSION="" 
WORKLOAD=""
DEBUG=0
SLEEP=""
RUNNING=1
COUNT=1 
DIR_FTRACE=/sys/kernel/debug/tracing/
FTRACE_BUFSIZE_DEFAULT=1410
FTRACE_BUFSIZE=`expr ${FTRACE_BUFSIZE_DEFAULT} \* 1000` 

# signal handler
trap 'handler' 2

# handler function
handler(){
    echo "SIG_INT signal accepted exit experiment!"
    if [ $DEBUG == 1 ]
    then
        echo 0 > ${PROC_DEBUG_ENABLE} 
    fi 
    echo 0 > ${PROC_SCAN_ENABLE}
    RUNNING=0    
    echo ${FTRACE_BUFSIZE_DEFAULT} > /sys/kernel/debug/tracing/buffer_size_kb
    kill -9 `pgrep son_log` 
}

# usage function
usage()
{
    echo ""
    echo "  usage : # ./expr.sh -s 10s -t v1 -w redis"   
    echo "        : # ./expr.sh -s 1m -d -t v2 -w 429mcf"
    echo ""
}


if [ $# -eq 0 ]
then 
    usage 
    exit 
fi

# option parsing
while getopts w:t:s:d opt 
do
    case $opt in 
        w)
            WORKLOAD=$OPTARG
            ;;
        t)
            VERSION=$OPTARG
            ;;
        s)
            SLEEP=$OPTARG
            ;;
        d)
            echo "debug option is enabled ! "

            if [ ${DEBUG} == 1 ] 
            then
                echo 1 > ${PROC_DEBUG_ENABLE}
            fi

            DEBUG=1            
            ;;
        *)
            usage 
            exit 0
            ;;
    esac
done

if [ -z $SLEEP ] || [ -z $VERSION ] || [ -z $WORKLOAD ] 
then 
    usage
    exit 0
fi

echo "increase buffer size to ${FTRACE_BUFSIZE}"
echo ${FTRACE_BUFSIZE} > ${DIR_FTRACE}/buffer_size_kb
./son_log.sh -t $VERSION -w $WORKLOAD &

while [ ${RUNNING} == 1 ];
do  
    echo "wakeup and do experiment! - $[COUNT] "
    echo 1 > ${PROC_SCAN_ENABLE} 
    echo "time to sleep for ${SLEEP}"
    sleep ${SLEEP}
    COUNT=`expr ${COUNT} + 1` 
done 
