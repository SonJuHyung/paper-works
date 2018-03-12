#ifndef _SON_H_
#define _SON_H_

#include <linux/mm.h>

#define SUCCESS         0
#define SON_ENABLE      1
#define SON_DISABLE     0

extern atomic_t son_refscan_enable;	
extern atomic_t son_debug_enable;	

typedef enum {
	PB_FREE,    
    PB_INUSE,
    PB_UNMOVABLE = PB_INUSE,
    PB_MOVABLE,
} scan_result_t;

extern wait_queue_head_t son_scand_wait; 

struct son_scand_control {
    unsigned long pfn_start;
    // scan 가능한 min pfn
    unsigned long pfn_end;
    // scan 가능한 max pfn 
    unsigned long pb_scanned;
    // zone 처음부터 시작하여 scan 한 page 의 수 
    unsigned long pb_free;
    // scan 결과 buddy 에서 관리되는 page 
    unsigned long pb_inuse;
    // scan 결과 free 가 아닌 page 
    int status;
    // scanning status;
};

#endif
