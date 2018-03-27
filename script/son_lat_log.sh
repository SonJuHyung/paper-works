#!/bin/bash 

DIR_YCSB=$(pwd)/.. 
DIR_DATA=""

VERSION="" 
WORKLOAD="" 
LAT=""
SLEEP=0
RUNNING=1
COUNT=1 

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
    echo "  usage : # ./son_lat_log.sh -s 10s -w redis -t v1"   
    echo "        : # ./son_lat_log.sh -s 1m  -w mongodb -t v2"
    echo ""
}

info(){
    echo ""
    echo "experiment info" 
    echo " - workload       : ${WORKLOAD}" 
    echo " - sleep interval : ${SLEEP}" 
    echo ""
}

if [ $# -eq 0 ]
then 
    usage 
    exit 
fi

# option parsing
while getopts w:t:s: opt 
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
        *)
            usage 
            exit 0
            ;;
    esac
done

if [ -z ${VERSION} ] || [ -z ${WORKLOAD} ]
then 
    usage
    exit 0
fi

DIR_DATA=${DIR_YCSB}/db/${WORKLOAD}/lat
FILE_DATA=${DIR_DATA}/lat_result_${WORKLOAD}_${VERSION}.txt

if [ -e ${FILE_DATA} ]
then 
    rm -rf ${FILE_DATA}
fi

info

PRE_LAT=0
PRE_MAX=1
CUR_LAT=0
CUR_MAX=1
LAT=0

while [ ${RUNNING} == 1 ];
do  
    LAT_CONTEXT=$(redis-cli latency latest | grep -v "fast-command" | awk '/command/ {for(i=1; i<=3; i++) {getline; print}}' | sed -n '2,3p')
    CUR_LAT=$(echo ${LAT_CONTEXT} | cut -d " " -f 1)
    CUR_MAX=$(echo ${LAT_CONTEXT} | cut -d " " -f 2)

    if [ ! -z ${CUR_LAT} ] && [ ! -z ${CUR_MAX} ]
    then
    if [ "$CUR_MAX" -gt "$PRE_MAX" ]
        then 
            echo "${COUNT},${CUR_MAX},${CUR_MAX}"
            echo "${COUNT},${CUR_MAX},${CUR_MAX}" >> ${FILE_DATA} 
            COUNT=`expr ${COUNT} + 1` 

        else
            echo "${COUNT},${CUR_LAT},${CUR_MAX}"
            echo "${COUNT},${CUR_LAT},${CUR_MAX}" >> ${FILE_DATA} 
            COUNT=`expr ${COUNT} + 1` 
        fi
    fi

    if [ ${SLEEP} != 0 ]
    then
        sleep ${SLEEP}
    fi
    PRE_LAT=${CUR_LAT}
    PRE_MAX=${CUR_MAX}
done 

