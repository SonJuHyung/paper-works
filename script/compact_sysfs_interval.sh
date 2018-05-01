#!/bin/bash 

DIR_EXPR=$(pwd)/.. 
DIR_DATA=${DIR_EXPR}/data/temp
COMPACT_ENABLE=/proc/sys/vm/compact_memory
REFSCAN_ENABLE=/proc/son/scan_refcount_enable
FILE_SYSFRAG=/sys/kernel/debug/extfrag/unusable_index  


VERSION="" 
WORKLOAD="" 
BEFORE_COMPACT=-1
AFTER_COMPACT=-1
COMPACT_CHAR="default"
REFSCAN_CHAR="default"
SLEEP=""
RUNNING=1
COUNT=1 
COMPACT=0 
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
    echo "  usage : # ./compact_sysfs_interval.sh -s 10s -t v1 -w vm_redis -r"   
    echo "        : # ./compact_sysfs_interval.sh -s 1m  -t v2 -w vm_mongodb -c"
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
while getopts w:t:s:cr opt 
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

if [ -z $SLEEP ] || [ -z $VERSION ] || [ -z $WORKLOAD ] 
then 
    usage
    exit 0
fi

FILE_DATA=${DIR_DATA}/frag_result_${COMPACT_CHAR}_${REFSCAN_CHAR}_${WORKLOAD}_${VERSION}.txt 

if [ -e ${FILE_DATA} ]
then 
    rm -rf ${FILE_DATA}
fi

# FRAG_NORM=0.7
# info
# echo ""
# echo "waiting until ufi reaches to ${FRAG_NORM} ..."
# echo ""
# while [ ${RUNNING} == 1 ]
# do
#     #if [ "$FRAG" '>' "$FRAG_NORM" | bc -l ] 
#     FRAG_CONTEXT=$(cat ${FILE_SYSFRAG} | grep Normal)
#     FRAG=$(echo $FRAG_CONTEXT | awk '{ split($0,arr," "); print(arr[14]); }')
#     STATUS=$(echo $FRAG'>'$FRAG_NORM | bc -l)
#     if [ $STATUS == 1  ]
#     then
#         echo ""
#         echo "ufi reached to ${FRAG_NORM}"
#         echo "${FRAG} is bigger than ${FRAG_NORM} start logging ..."
#         echo ""
#         RUNNING=0
#     fi
# done

RUNNING=1
    
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

    if [ ${REFSCAN} == 1 ]
    then
        echo "refcount scanning ..."
        echo 1 > ${REFSCAN_ENABLE}         
    fi

    # ufi logging version
    echo "${COUNT},${BEFORE_COMPACT},${AFTER_COMPACT}"
    echo "${COUNT},${BEFORE_COMPACT},${AFTER_COMPACT}" >> ${FILE_DATA}

    sleep ${SLEEP}
    COUNT=`expr ${COUNT} + 1` 
done 
