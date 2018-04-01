#!/bin/bash 

DIR_EXPR=$(pwd)/.. 
DIR_DATA=${DIR_EXPR}/data/ftrace 
DIR_FTRACE=/sys/kernel/debug/tracing 
VERSION="" 
WORKLOAD=""

FTRACE_OFF=0
FTRACE_ON=1
FTRACE_LS=0
FTRACE_FUNC_OFF="nop"
FTRACE_FUNC_ON="function"
FTRACE_PID=""

# signal handler
trap 'handler' 2

# handler function 
ftrace_unset(){
    echo ""
    echo "  unsetting ftrace parameters..."
    echo ""
    echo ${FTRACE_OFF} > ${DIR_FTRACE}/tracing_on
    echo ${FTRACE_FUNC_OFF} > ${DIR_FTRACE}/current_tracer
    echo > ${DIR_FTRACE}/set_ftrace_filter 
    echo > ${DIR_FTRACE}/set_ftrace_pid
    echo > ${DIR_FTRACE}/trace   
}

ftrace_set(){
    echo ""
    echo "  setting ftrace parameters..."
    echo ""
    echo > ${DIR_FTRACE}/trace   
    echo ${FTRACE_PID} > ${DIR_FTRACE}/set_ftrace_pid
    echo ${FTRACE_FUNC_TARGET} > ${DIR_FTRACE}/set_ftrace_filter
    echo ${FTRACE_FUNC_ON} > ${DIR_FTRACE}/current_tracer
    echo ${FTRACE_ON} > ${DIR_FTRACE}/tracing_on
}

handler(){
    echo "SIG_INT signal accepted exit experiment!" 
    ftrace_unset
}

# usage function
usage()
{
    echo "  usage : # ./son_ftrace_func.sh -f __alloc_pages_slowpath -w redis -p redis -v v1"   
    echo "        : # ./son_ftrace_func.sh -f __mmput -w mongodb -p mongod -v v2"
    echo "        : # ./son_ftrace_func.sh -l"
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

# option parsing
while getopts f:v:w:p:l opt 
do
    case $opt in 
        w)
            WORKLOAD=$OPTARG
            echo "  expr workload : ${WORKLOAD}"
            ;;
        p)
            FTRACE_PID=$(pgrep ${OPTARG})
            if [ $? != 0 ]
            then 
                echo "  error : ${OPTARG} process doesn't exist" 
                echo > ${DIR_FTRACE}/set_ftrace_pid 
                usage 
                exit 0
            else 
                echo "  workload pid  : ${FTRACE_PID}"
            fi
            ;;

        f)
            FTRACE_FUNC_TARGET=$(cat ${DIR_FTRACE}/available_filter_functions | grep ${OPTARG})
            if [ $? != 0 ]                               
            then                
                echo "  error : ${FTRACE_FUNC_TARGET} function doesn't exist in available tracer" 
                echo > ${DIR_FTRACE}/set_ftrace_filter
                usage
                exit 0                
            else 
                echo "  target func   : ${FTRACE_FUNC_TARGET}"
            fi
            ;;
        v)
            VERSION=$OPTARG 
            echo "  exper ver     : ${VERSION}"
            ;;

        l)
            FTRACE_LS=1
            ;;            
        *)
            usage 
            exit 0
            ;;
    esac
done

if [ ${FTRACE_LS} == 1 ]
then
    cat ${DIR_FTRACE}/available_filter_functions
    exit 0
fi

if [ -z $FTRACE_FUNC_TARGET ] || [ -z $VERSION ] || [ -z $WORKLOAD ] || [ -z ${FTRACE_PID} ]
then 
    usage
    exit 0
fi

FILE_DATA=${DIR_DATA}/ftrace_func_result_${FTRACE_FUNC_TARGET}_${WORKLOAD}_${VERSION}.txt 
if [ -e ${FILE_DATA} ]
then 
    rm -rf ${FILE_DATA}
fi

ftrace_set
cat ${DIR_FTRACE}/trace_pipe > ${FILE_DATA}
