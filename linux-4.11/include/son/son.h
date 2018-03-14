#ifndef _SON_H_
#define _SON_H_

#include <linux/mm.h>

#define SUCCESS         0
#define SON_ENABLE      1
#define SON_DISABLE     0

extern atomic_t son_refscan_enable;	
extern atomic_t son_debug_enable;	

typedef enum {
	PB_FREE,                    // 0
    PB_INUSE,                   // 1
    PB_UNMOVABLE = PB_INUSE,    // 1
    PB_MOVABLE,                 // 2
    PB_RECLAIMABLE,             // 3
    PB_HIGHATOMIC,              // 4
    PB_ISOLATE,                 // 5
    PB_INVALID,                 // 6
    PB_STATNUM                  // 7 MAX ENTRY
} scan_result_t;

extern wait_queue_head_t son_scand_wait; 

struct son_scand_control {
    unsigned long pfn_start;
    // scan 가능한 min pfn
    unsigned long pfn_end;
    // scan 가능한 max pfn 
    unsigned long pb_scanned;
    // zone 처음부터 시작하여 scan 한 page 의 수 
    unsigned long pb_stat[PB_STATNUM];
    int status;
    // scanning status;
};

#endif
