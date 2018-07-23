#!/bin/bash 

DIR_EXPR=$(pwd)/.. 
DIR_DATA=${DIR_EXPR}/data/pbstat
PBSTATINFO_DATA=/proc/son/pbstat_info_show_raw
TARGET_ZONE="Normal"

WORKLOAD="" 
VERSION="" 
TYPE=""
SLEEP=""
RUNNING=1
COUNT=1 

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
    echo "  usage : # ./pbstat.sh -w vm_redis -t thp -s 10s -v v1"
    echo "        : # ./pbstat.sh -w vm_mongodb -t nhp -s 1m -v v2"
    echo ""
}

info(){
    echo ""
    echo "experiment info" 
    echo " - workload       : ${WORKLOAD}" 
    echo " - thp type       : ${TYPE}" 
    echo " - sleep interval : ${SLEEP}"
    echo " - version        : ${VERSION}" 
    echo ""
}

if [ $# -eq 0 ]
then 
    usage 
    exit 
fi

# option parsing
while getopts w:t:v:s: opt 
do
    case $opt in         
        w)
            WORKLOAD=$OPTARG
            ;;
        t)
            TYPE=$OPTARG
            ;;
        s)
            SLEEP=$OPTARG
            ;;
        v)
            VERSION=$OPTARG
            ;;
        *)
            usage 
            exit 0
            ;;
    esac
done

if [ -z $WORKLOAD ] || [ -z $TYPE ] || [ -z $SLEEP ] || [ -z $VERSION ] 
then 
    echo "${WORKLOAD}, ${TYPE}, ${SLEEP}, ${VERSION}"
    usage
    exit 0
fi

FILE_DATA=${DIR_DATA}/pbstat_result_${WORKLOAD}_${TYPE}_${SLEEP}_${VERSION}.txt 

if [ -e ${FILE_DATA} ]
then 
    rm -rf ${FILE_DATA}
fi

info

echo "start logging to ${FILE_DATA} ..."
echo ""

RUNNING=1
    
while [ ${RUNNING} == 1 ];
do  
    PBSTAT_CONTEXT=$(cat ${PBSTATINFO_DATA} | grep Normal)

    echo ${PBSTAT_CONTEXT}
    echo ${PBSTAT_CONTEXT} >> ${FILE_DATA}

    sleep ${SLEEP}
    COUNT=`expr ${COUNT} + 1` 
done 

