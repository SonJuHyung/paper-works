#!/bin/bash 

TRACER="nop"

echo ${TRACER} > /sys/kernel/debug/tracing/current_tracer
echo > /sys/kernel/debug/tracing/set_ftrace_pid
echo > /sys/kernel/debug/tracing/set_ftrace_filter 

