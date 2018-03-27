#!/bin/bash 

DIR_EXPR=$(pwd)/.. 
DIR_DATA=${DIR_EXPR}/data/compact
COMPACT_ENABLE=/proc/sys/vm/compact_memory
 FILE_SYSFRAG=/sys/kernel/debug/extfrag/unusable_index 

VERSION="" 
WORKLOAD="" 
BEFORE_COMPACT=-1
AFTER_COMPACT=-1
COMPACT_CHAR="default"
SLEEP=""
RUNNING=1
COUNT=1 
COMPACT=0

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
    echo "  usage : # ./son_compact.sh -s 10s -t v1 -w vm_redis"   
    echo "        : # ./son_compact.sh -s 1m  -t v2 -w vm_mongodb -c"
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
while getopts w:t:s:c opt 
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
        c)
            COMPACT=1 
            COMPACT_CHAR="compact"
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

FILE_DATA=${DIR_DATA}/frag_result_${WORKLOAD}_${COMPACT_CHAR}_${VERSION}.txt 
if [ -e ${FILE_DATA} ]
then 
    rm -rf ${FILE_DATA}
fi

info

while [ ${RUNNING} == 1 ];
do  
    echo "wakeup and do experiment! - $[COUNT] "
    FRAG_CONTEXT=$(cat ${FILE_SYSFRAG} | grep Normal)
    BEFORE_COMPACT=$(echo $FRAG_CONTEXT | awk '{ split($0,arr," "); print(arr[14]); }')
    if [ ${COMPACT} == 1 ]
    then 
        echo "compact memory ..."
        echo 1 > ${COMPACT_ENABLE} 
        FRAG_CONTEXT=$(cat ${FILE_SYSFRAG} | grep Normal)
        AFTER_COMPACT=$(echo $FRAG_CONTEXT | awk '{ split($0,arr," "); print(arr[14]); }')
    fi 
    echo "${COUNT},${BEFORE_COMPACT},${AFTER_COMPACT}"
    echo "${COUNT},${BEFORE_COMPACT},${AFTER_COMPACT}" >> ${FILE_DATA}

    sleep ${SLEEP}
    COUNT=`expr ${COUNT} + 1` 
done 
