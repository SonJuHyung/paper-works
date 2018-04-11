#ifndef _SON_H_
#define _SON_H_

//#include <linux/mm.h>

#define SON_SUCCESS         0
#define SON_ENABLE      1
#define SON_DISABLE     0

extern spinlock_t son_scan_refcount_list_lock;

extern atomic_t son_scan_pbstate_enable;	
extern atomic_t son_debug_enable;	
extern atomic_t son_scan_refcount_enable;

extern wait_queue_head_t son_scand_pbstate_wait; 
extern wait_queue_head_t son_scand_refcount_wait;

typedef enum {
	PB_FREE,                    // 0
    PB_INUSE,                   // 1
    PB_UNMOVABLE = PB_INUSE,    // 1
    PB_MOVABLE,                 // 2
    PB_RECLAIMABLE,             // 3
    PB_HIGHATOMIC,              // 4
    PB_ISOLATE,                 // 5
    PB_INVALID,                 // 6 
    PB_COMPACT,                 // 7
    PB_MAXENTRY                 // 8 MAX ENTRY
} scan_result_t;

struct son_scand_control {
    unsigned long pfn_start;
    // scan 가능한 min pfn
    unsigned long pfn_end;
    // scan 가능한 max pfn 
    unsigned long pb_scanned;
    // zone 처음부터 시작하여 scan 한 page 의 수 
    unsigned long pb_stat[PB_MAXENTRY];
    int status;
    // scanning status;
};

typedef struct son_scand_refcount_stats {
    unsigned long idle_bpage_count;
    unsigned long total_bpage_count;
    unsigned long idle_hpage_count;
    unsigned long total_hpage_count;
}son_scand_stats_t;


void son_kthread_refcount_add_entry(struct mm_struct *mm); 
void son_kthread_refcount_del_entry(struct mm_struct *mm);



#endif
