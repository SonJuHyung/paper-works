#!/bin/bash 
FUNC="compact_node_monitor"
#FUNC="son_compact_node"
TRACER="function_graph"

echo ${FUNC} > /sys/kernel/debug/tracing/set_ftrace_filter 
echo ${TRACER} > /sys/kernel/debug/tracing/current_tracer
