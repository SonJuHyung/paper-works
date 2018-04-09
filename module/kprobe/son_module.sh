#!/bin/bash 
FILE_MODULE=""
FILE_MODULE_PREFIX=""
DIR_EXPR=$(pwd)/../.. 
DIR_DATA=${DIR_EXPR}/data/module
DIR_FTRACE=/sys/kernel/debug/tracing 

WORKLOAD=""
VERSION=""

# signal handler
trap 'handler' 2

# handler function 
handler(){
    echo "  SIG_INT signal accepted exit experiment!" 
    echo "  unloading ${FILE_MODULE} module..."
    $(lsmod | grep ${FILE_MODULE_PREFIX} > /dev/null)
    if [ $? != 0 ]
    then
        echo "  error : ${FILE_MODULE} module doesn't exist" 
    else
        rmmod ${FILE_MODULE}
        echo "  done!"
    fi

    FRAG_CONTEXT=$(cat /sys/kernel/debug/extfrag/unusable_index | grep Normal)
    FRAG=$(echo $FRAG_CONTEXT | awk '{ split($0,arr," "); print(arr[14]); }')

    echo "  logging fragmentation index..."
    if [ -e ${FILE_DATA} ]
    then
        echo $FRAG >> ${FILE_DATA}
    else
        echo "  error : log file doesn't exist!"
    fi
    echo "  done!"
    echo "  exiting script!!"
    echo ""
    exit 0
}


# usage function
usage()
{
    echo "  usage : # ./son_module.sh -m son_jprobe.ko -w redis -v v1"   
    echo "        : # ./son_module.sh -m son_jprobe.ko -w mongodb -v v2"
    echo ""
}

if [ $# -eq 0 ]
then 
    usage 
    exit 
fi

if [ $(id -u) -ne 0 ] 
then 
    exec sudo bash "$0" "$@"
fi

make clean
make

# option parsing
while getopts v:w:m: opt 
do
    case $opt in 
        m)
            if [ -e ${OPTARG} ]
            then
                FILE_MODULE=${OPTARG}               
                FILE_MODULE_PREFIX=$(echo ${FILE_MODULE} | awk '{ split($0,arr,".ko"); print(arr[1]); }')
                echo "  module name    : ${FILE_MODULE_PREFIX}"
            else
                echo "  error : ${OPTARG} module doesn't exist" 
                exit 0
            fi
            ;;
        w)
            WORKLOAD=$OPTARG
            echo "  expr workload  : ${WORKLOAD}"
            ;;
        v)
            VERSION=$OPTARG 
            echo "  exper ver      : ${VERSION}"
            ;;
        *)
            usage 
            exit 0
            ;;
    esac
done

if [ -z $VERSION ] || [ -z $WORKLOAD ] || [ -z $FILE_MODULE ]
then 
    usage
    exit 0
fi

FILE_DATA=${DIR_DATA}/probe_raw_result_${WORKLOAD}_${VERSION}.txt  
echo ""
echo "  loading ${FILE_MODULE} module..." 

$(lsmod | grep ${FILE_MODULE_PREFIX} > /dev/null)
if [ $? != 0 ]
then 
    insmod ${FILE_MODULE}
    echo "  done!"
else
    echo "  error : ${FILE_MODULE} module exists" 
    echo "          unload ${FILE_MODULE} and exit expr!!" 
    rmmod ${FILE_MODULE}
    exit 0
fi


echo "  start logging to ${FILE_DATA}"

#./son_module_log.sh -w ${WORKLOAD} -v ${VERSION} &
cat ${DIR_FTRACE}/trace_pipe > ${FILE_DATA} 

