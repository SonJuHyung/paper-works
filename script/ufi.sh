#!/bin/bash 

SLEEP=""
RUNNING=1

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
    echo "  usage : # ./ufi.sh -s 10s"   
    echo "        : # ./ufi.sh -s 1m"
    echo ""
}


# option parsing
while getopts s: opt 
do
    case $opt in 
        s)
            SLEEP=$OPTARG
            ;;
        *)
            usage 
            exit 0
            ;;
    esac
done

if [ -z ${SLEEP} ]
then 
    usage
    exit 0
fi

COUNT=1
FILE_DATA="test.txt"

if [ -e ${FILE_DATA} ]
then
    rm -rf ${FILE_DATA}
fi

while [ ${RUNNING} == 1 ];
do  
    FRAG_CONTEXT=$(cat /sys/kernel/debug/extfrag/unusable_index | grep Normal)   
    FRAG=$(echo $FRAG_CONTEXT | awk '{ split($0,arr," "); print(arr[14]); }')

    echo "${COUNT},${FRAG},"
    echo "${COUNT},${FRAG}," >> ${FILE_DATA}

    if [ ${SLEEP} != 0 ]
    then
        sleep ${SLEEP}
    fi
    COUNT=`expr ${COUNT} + 1`
done 

