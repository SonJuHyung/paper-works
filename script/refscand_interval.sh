#!/bin/bash 

REFSCAN_ENABLE=/proc/son/scan_refcount_enable

SLEEP=""
RUNNING=1
COUNT=1 
REFSCAN=0

# signal handler
trap 'handler' 2

# handler function
handler(){
    echo "SIG_INT signal accepted exit experiment!"    
    RUNNING=0    
}

# usage function
usage()
{
    echo ""
    echo "  usage : # ./refscand_interval.sh -s 10s -r"   
    echo "        : # ./refscand_interval.sh -s 1m -r"
    echo ""
}

info(){
    echo ""
    echo "experiment info" 
    echo " - workload       : ${WORKLOAD}" 
    echo " - sleep interval : ${SLEEP}" 
    echo " - compact        : ${COMPACT}" 
    echo ""
}

if [ $# -eq 0 ]
then 
    usage 
    exit 
fi

# option parsing
while getopts s:r opt 
do
    case $opt in 
        s)
            SLEEP=$OPTARG
            ;;
        r)
            REFSCAN=1
            REFSCAN_CHAR="refscan"
            ;;
        *)
            usage 
            exit 0
            ;;
    esac
done

if [ -z $SLEEP ]
then 
    usage
    exit 0
fi
    
while [ ${RUNNING} == 1 ];
do  
    echo "wakeup and do experiment! - $[COUNT] "

    if [ ${REFSCAN} == 1 ]
    then
        echo "refcount scanning ..."
        echo 1 > ${REFSCAN_ENABLE}         
    fi

    sleep ${SLEEP}
    COUNT=`expr ${COUNT} + 1` 
done 
