#!/bin/bash 

#FUNC="compact_node"
FUNC="son_compact_node"
TRACER="function_graph"
PID=$(pgrep compact_run)

echo ${PID} > /sys/kernel/debug/tracing/set_ftrace_pid
echo ${FUNC} > /sys/kernel/debug/tracing/set_ftrace_filter 
echo ${TRACER} > /sys/kernel/debug/tracing/current_tracer
