#!/bin/bash 

redis-cli config set stop-writes-on-bgsave-error no
redis-cli config set save "" 
redis-cli config set dir /home/son/expr/redis/
