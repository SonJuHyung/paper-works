#!/bin/bash 

DIR_EXPR=$(pwd)/.. 
DIR_DATA=${DIR_EXPR}/data 
PROC_SCAN_ENABLE=/proc/son/scan_ref_enable 
PROC_DEBUG_ENABLE=/proc/son/debug
VERSION=""
DEBUG=0
SLEEP=""
RUNNING=1
COUNT=1

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
    kill -9 `pgrep son_log` 
}

# usage function
usage()
{
    echo ""
    echo "  usage : # ./expr.sh -s 10s -t v1"   
    echo "        : # ./expr.sh -s 1m -d -t v2"
    echo ""
}


if [ $# -eq 0 ]
then 
    usage 
    exit 
fi

# option parsing
while getopts t:s:d opt 
do
    case $opt in 
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

if [ -z $SLEEP ] || [ -z $VERSION ] 
then 
    usage
    exit 0
fi

./son_log.sh -t $VERSION &

while [ ${RUNNING} == 1 ];
do  
    echo "wakeup and do experiment! - $[COUNT] "
    echo 1 > ${PROC_SCAN_ENABLE} 
    echo "time to sleep for ${SLEEP}"
    sleep ${SLEEP}
    COUNT=`expr ${COUNT} + 1` 
done 
