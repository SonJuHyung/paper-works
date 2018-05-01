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
    + fs/proc/internal.h

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
  
3. obtained free paage block during compaction.
  + obtained page block during sysfs memory compaction.
  + output in ftrace ring buffer 
  + log 
    + mm/internal.h migrate.c compaction.c

4. page block utilization information.    
  + log page block utilization in radix tree node.
  + isolate page block which contains unmovable pages from compaction target.
  + linked list per page block utilization(BLUE, GREEN, YELLOW, RED)
  + log 
    + include/linux/mm_types.h mmzone.h 
    + include/son/son.h
    + son/son_scan.c
    + mm/memory.c
    + mm/page_alloc.c

5. TODO 
  + make kcompactd to act before DC.
  + make kcompactd compaction algorithm to use page block utilize information.
  + make threshold vale similar to WMARK value.

--------------------------------------


