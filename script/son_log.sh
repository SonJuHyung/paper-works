#!/bin/bash 

DIR_EXPR=$(pwd)/..
DIR_DATA=${DIR_EXPR}/data 
DIR_FTRACE=/sys/kernel/debug/tracing/ 
PROC_SCAN_ENABLE=/proc/son/scan_ref_enable 
FILE_DATA=""
VERSION=""

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
    echo "  usage : # ./son_log.sh -t v1"   
    echo "        : # ./son_log.sh -t v2"
    echo ""
}


if [ $# -eq 0 ]
then 
    usage 
    exit 
fi

# option parsing
while getopts t: opt 
do
    case $opt in
        t)
            VERSION=$OPTARG
            ;;
        *)
            usage 
            exit 0
            ;;
    esac
done

if [ -z $VERSION ] 
then 
    usage
    exit 0
fi

FILE_DATA=${DIR_DATA}/pbstate_result_${VERSION}.txt 
echo > ${DIR_FTRACE}/trace
cat ${DIR_FTRACE}/trace_pipe | awk '{ split($0,arr,":"); printf("%s\n",arr[3]); }'> ${FILE_DATA} 

