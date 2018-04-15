#!/bin/bash 

DIR_EXPR=$(pwd)/.. 
DIR_DATA=${DIR_EXPR}/data/ftrace 
VERSION="" 
WORKLOAD=""

DIR_FTRACE=/sys/kernel/debug/tracing 
DIR_FTRACE_EVENTS=${DIR_FTRACE}/events

FTRACE_MODE=0
FTRACE_MODE_NAME=("func" "event")
FTRACE_OFF=0
FTRACE_ON=1
FTRACE_LS=0
FTRACE_PID=""

FTRACE_EVENT_DIR=""
FTRACE_EVENT_EVENT=""

# signal handler
trap 'handler' 2
    
# handler function 
handler(){
    echo "  SIG_INT signal accepted exit experiment!" 
    echo "  unsetting ftrace..."
    if [ ${FTRACE_MODE} == 1 ]
    then
        ftrace_event_unset
    else
        ftrace_func_unset 
    fi
    echo "  done!"

    FRAG_CONTEXT=$(cat /sys/kernel/debug/extfrag/unusable_index | grep Normal)
    FRAG=$(echo $FRAG_CONTEXT | awk '{ split($0,arr," "); print(arr[14]); }')

    echo "  logging fragmentation index..."
    echo $FRAG >> ${FILE_DATA}
    echo "  done!"
    echo "  exiting script!!"
    echo ""
    exit 0
}

# usage function
usage()
{
    echo "  usage : # ./son_ftrace.sh -f __alloc_pages_slowpath -w redis -p redis-server -v v1"   
    echo "        : # ./son_ftrace.sh -f __mmput -w mongodb -p mongod -v v2"
    echo "        : # ./son_ftrace.sh -w redis -p redis-server -e -v v1"
    echo "        : # ./son_ftrace.sh -l"
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
while getopts f:v:w:p:e:l opt 
do
    case $opt in 
        w)
            WORKLOAD=$OPTARG
            echo "  expr workload      : ${WORKLOAD}"
            ;;
        p)
            $(pgrep ${OPTARG} > /dev/null)
            if [ $? != 0 ]
            then 
                echo "  error : ${OPTARG} process doesn't exist" 
                echo > ${DIR_FTRACE}/set_ftrace_pid 
                usage 
                exit 0
            else 
                FTRACE_PID=${OPTARG}
                echo "  workload pid       : ${FTRACE_PID}"
            fi
            ;;

        f)
            $(cat ${DIR_FTRACE}/available_filter_functions | grep ${OPTARG} > /dev/null)
            if [ $? != 0 ]                               
            then                
                echo "  error : ${FTRACE_FUNC_TARGET} function doesn't exist in available tracer" 
                echo > ${DIR_FTRACE}/set_ftrace_filter
                usage
                exit 0                
            else 
                FTRACE_FUNC_TARGET=${OPTARG}
                echo "  target func        : ${FTRACE_FUNC_TARGET}"
            fi
            ;;
        v)
            VERSION=$OPTARG 
            echo "  exper ver          : ${VERSION}"
            ;;

        e)
            FTRACE_EVENT_DIR=$(echo ${OPTARG} | awk '{ split($0,arr,":"); print(arr[1]); }')
            FTRACE_EVENT_EVENT=$(echo ${OPTARG} | awk '{ split($0,arr,":"); print(arr[2]); }')

            if [ -e ${DIR_FTRACE_EVENTS}/${FTRACE_EVENT_DIR} ]
            then
                echo "  target event dir   : ${FTRACE_EVENT_DIR}"
            else
                echo "  error : ${FTRACE_EVENT_DIR} function doesn't exist in available event dir" 
                usage
                exit 0                
            fi

            if [ -e ${DIR_FTRACE_EVENTS}/${FTRACE_EVENT_DIR}/${FTRACE_EVENT_EVENT} ]
            then                
                echo "  target event       : ${FTRACE_EVENT_EVENT}"
            else 
                echo "  error : ${FTRACE_EVENT_EVENT} function doesn't exist in available event list" 
                usage
                exit 0                
            fi 

            FTRACE_MODE=1
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

if [ -z $VERSION ] || [ -z $WORKLOAD ] || [ -z ${FTRACE_PID} ]
then 
    usage
    exit 0
fi

if [ ${FTRACE_MODE} == 1 ]
then 

    if [ -z ${FTRACE_EVENT_DIR} ] || [ -z ${FTRACE_EVENT_EVENT} ]
    then 
        usage
        exit 0
    fi

    echo ""    
    echo "  start ftrace event tracing experiment!!"

    FTRACE_EVENT_EVENT_FILTER="common_pid == ${FTRACE_PID}"

    # ftrace event tracing
    ftrace_event_unset(){
        echo "  unsetting ftrace event tracing parameters..."
        echo ${FTRACE_OFF} > ${DIR_FTRACE}/tracing_on 
        echo ${FTRACE_OFF} > ${DIR_FTRACE_EVENTS}/${FTRACE_EVENT_DIR}/${FTRACE_EVENT_EVENT}/enable
        echo ${FTRACE_OFF} > ${DIR_FTRACE_EVENTS}/${FTRACE_EVENT_DIR}/${FTRACE_EVENT_EVENT}/filter
        echo > ${DIR_FTRACE}/trace   
        echo "  done!"

    }

    ftrace_event_set(){
        echo "  setting ftrace event tracing parameters..."
        echo > ${DIR_FTRACE}/trace  
        echo ${FTRACE_EVENT_EVENT_FILTER} > ${DIR_FTRACE_EVENTS}/${FTRACE_EVENT_DIR}/${FTRACE_EVENT_EVENT}/filter
        echo ${FTRACE_ON} > ${DIR_FTRACE_EVENTS}/${FTRACE_EVENT_DIR}/${FTRACE_EVENT_EVENT}/enable
        echo ${FTRACE_ON} > ${DIR_FTRACE}/tracing_on    
        echo "  done!"
    }

    FILE_DATA=${DIR_DATA}/${FTRACE_MODE_NAME[${FTRACE_MODE}]}/ftrace_raw_${FTRACE_MODE_NAME[${FTRACE_MODE}]}_result_${FTRACE_EVENT_DIR}_${FTRACE_EVENT_EVENT}_${WORKLOAD}_${VERSION}.txt 
    if [ -e ${FILE_DATA} ]
    then 
        rm -rf ${FILE_DATA}
    fi

    ftrace_event_set 

    echo "  start logging to ${FILE_DATA}"
    cat ${DIR_FTRACE}/trace_pipe > ${FILE_DATA} 

#    cat $1 | awk '{ split($0,parse1,":"); funcname=parse1[2]; split(parse1[3],parse2," "); split(parse2[2],_pfn,"="); pfn=_pfn[2]; split(parse2[3],_order,"="); order=_order[2]; split(parse2[4],_mtype,"="); mtype=_mtype[2]; printf("%s,%s,%s\n",funcname,pfn,order,mtype);  }'

else

    if [ ${FTRACE_LS} == 1 ]
    then
        cat ${DIR_FTRACE}/available_filter_functions
        exit 0
    fi

    if [ -z $FTRACE_FUNC_TARGET ] 
    then 
        usage
        exit 0
    fi

    echo ""    
    echo "  start ftrace function tracing experiment!!"

    FTRACE_FUNC_OFF="nop"
    FTRACE_FUNC_ON="function"

    # ftrace function tracing 
    ftrace_func_unset(){
        echo "  unsetting ftrace func tracing parameters..."
        echo ${FTRACE_OFF} > ${DIR_FTRACE}/tracing_on 
        echo ${FTRACE_FUNC_OFF} > ${DIR_FTRACE}/current_tracer
        echo > ${DIR_FTRACE}/set_ftrace_filter 
        echo > ${DIR_FTRACE}/set_ftrace_pid
        echo > ${DIR_FTRACE}/trace   
        echo "  done!"
    }

    ftrace_func_set(){
        echo "  setting ftrace func tracing parameters..."
        echo > ${DIR_FTRACE}/trace   
        echo ${FTRACE_PID} > ${DIR_FTRACE}/set_ftrace_pid
        echo ${FTRACE_FUNC_TARGET} > ${DIR_FTRACE}/set_ftrace_filter 
        echo ${FTRACE_FUNC_ON} > ${DIR_FTRACE}/current_tracer
        echo ${FTRACE_ON} > ${DIR_FTRACE}/tracing_on    
        echo "  done!"
    }

    FILE_DATA=${DIR_DATA}/${FTRACE_MODE_NAME[${FTRACE_MODE}]}/ftrace_raw_${FTRACE_MODE_NAME[${FTRACE_MODE}]}_result_${FTRACE_FUNC_TARGET}_${WORKLOAD}_${VERSION}.txt 
    if [ -e ${FILE_DATA} ]
    then 
        rm -rf ${FILE_DATA}
    fi

    ftrace_func_set

    echo "  start logging to ${FILE_DATA}"
    cat ${DIR_FTRACE}/trace_pipe > ${FILE_DATA} 
fi


