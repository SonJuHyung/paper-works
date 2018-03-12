Paper work updates
======================================

1. procfs 
  + debug option
    + echo 1 > /proc/son/debug
  + page block usage option 
    + echo 1 > /proc/son/scan_ref_enable 

2. page block usage tracking kernel thread 
  + can be on/off by procfs 
  + linux/son/

3. TODO 
  + page block reference count tracking 
  + kcompactd optimization 
  + bloom filter implement
--------------------------------------


