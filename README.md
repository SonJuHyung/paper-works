Paper work updates
======================================

1. procfs 
  + debug option
    + echo 1 > /proc/son/debug
  + can be on/off by procfs 
    + echo 1 > /proc/son/scan_pbstate_enable

2. page block usage tracking kernel thread  
  + debug option
    + echo 1 > /proc/son/debug
  + can be on/off by procfs 
    + echo 1 > /proc/son/scan_refcount_enable

3. TODO 
  + kcompactd optimization 
  + per page block utilization b+ tree
  + make LRU inactive list to consider b+ tree 
  + bloom filter implement  

4. log 
 + son/son_proc.c son/son_scan.c 
 + kernel/fork.c 
   + init_mm() 
   + ``__mmput()``
 + 
 + fs/proc/internal.h - file_operations
--------------------------------------


