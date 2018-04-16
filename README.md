Paper work updates
======================================

1. page block's used percentage tracking kernel thread
  + in-use pages percentage in page block.
  + debug option
    + echo 1 > /proc/son/debug
  + can be on/off by procfs 
    + echo 1 > /proc/son/scan_pbstate_enable 
  + log 
    + son/son_proc.c son_scan.c
    + proc/internal.h

2. reference counting kernel thread  
  + per-page reference counting.
  + debug option
    + echo 1 > /proc/son/debug
  + can be on/off by procfs 
    + echo 1 > /proc/son/scan_refcount_enable
  + log
    + son/son_proc.c son_scan.c
    + include/linux/mm_types.h mmzone.h page_idle.h
    + include/son/son.h
    + kernel/fork.c
    + mm/memory.c page_alloc.c
  

3. kcompactd's obtained free paage block 
  + obtained page block during sysfs memory compaction.
  + output in ftrace ring buffer 
  + log 
    + mm/internal.h migrate.c compaction.c

3. TODO 
  + kcompactd optimization 
  + per page block utilization b+ tree
  + make LRU inactive list to consider b+ tree 
  + bloom filter implement  

--------------------------------------


