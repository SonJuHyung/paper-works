RUNNING=1
redis-cli config set latency-monitor-threshold 1 
redis-cli latency reset
while [ ${RUNNING} == 1 ];
do
    redis-cli latency latest 
    sleep "1s"
done
