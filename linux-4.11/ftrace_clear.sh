#!/bin/bash 

TRACER="nop"

echo > /sys/kernel/debug/tracing/set_ftrace_filter 
echo ${TRACER} > /sys/kernel/debug/tracing/current_tracer
