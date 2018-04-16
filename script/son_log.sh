#!/bin/bash 

DIR_EXPR=$(pwd)/..
DIR_DATA=${DIR_EXPR}/data/pbusage_new
DIR_FTRACE=/sys/kernel/debug/tracing/ 
PROC_SCAN_ENABLE=/proc/son/scan_pbstate_enable 
FILE_DATA=""
VERSION="" 
WORKLOAD=""

trap 'handler' 2

# handler function
handler(){
    echo 0 > ${PROC_SCAN_ENABLE}
    exit 
}

# usage function
usage()
{
    echo ""
    echo "  usage : # ./son_log.sh -t v1 -w redis"
    echo "        : # ./son_log.sh -t v2 -w mcf429"
    echo ""
}


if [ $# -eq 0 ]
then 
    usage 
    exit 
fi

# option parsing
while getopts w:t: opt 
do
    case $opt in 
        w)
            WORKLOAD=$OPTARG
            ;;
        t)
            VERSION=$OPTARG
            ;;
        *)
            usage 
            exit 0
            ;;
    esac
done

if [ -z $VERSION ] || [ -z $WORKLOAD ] 
then 
    usage
    exit 0
fi

FILE_DATA=${DIR_DATA}/pbstate_result_${WORKLOAD}_${VERSION}.txt 
echo > ${DIR_FTRACE}/trace
cat ${DIR_FTRACE}/trace_pipe | awk '{ split($0,arr,":"); printf("%s\n",arr[3]); }'> ${FILE_DATA} 
#cat ${DIR_FTRACE}/trace_pipe > ${FILE_DATA} 

