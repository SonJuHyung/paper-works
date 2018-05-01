#ifndef _SON_H_
#define _SON_H_

//#include <linux/mm.h>
#include <linux/radix-tree.h>

#define SON_SUCCESS     0
#define SON_ENABLE      1
#define SON_DISABLE     0

// debug

/* 
 * VERSION 1.
 * paper's monitoring kernel thread related variables
 */

#define SON_PBSCAND_ENABLE      1
#define SON_REFSCAND_ENABLE     0
#define SON_DEBUG_ENABLE        0
#define SON_PBSTAT_ENABLE       1

/* page block usage info or state info which 
 * is used in son_pbscand kernel thread  */
typedef enum {                  /* page is */
	PB_FREE,                    /* 0 : free state(page is in buddy allcator)  */
    PB_ACTIVE,                  /* 1 : in ACTIVE LRU */
    PB_INACTIVE,                /* 2 : in INACTIVE LRU */
    PB_UNMOVABLE = PB_ACTIVE,   /* 1 : unmovable state */
    PB_MOVABLE,                 /* 2 : movable state*/
    PB_RECLAIMABLE,             /* 3 : reclaimable state */
    PB_HIGHATOMIC,              /* 4 : highatomic state */
    PB_ISOLATE,                 /* 5 : isolated state */
    PB_INVALID,                 /* 6 : invalid state(hole) */
    PB_COMPACT,                 /* 7 : scanned by compaction scanner */
    PB_MAXENTRY                 /* 8 : MAX ENTRY */
} scan_result_t;

struct son_scand_control {
    unsigned long pfn_start;
    /* available minimum page frame number. */
    unsigned long pfn_end;
    /* available maximum page frame number. */
    unsigned long pb_scanned;
    /* scanned page block count starting from zone's start pfn */
    unsigned long pb_stat[PB_MAXENTRY];
    /* scan_result_t information 
     * FIXME 
     *  - change data type to scan_result_t 
     */
    int status;
    // scanning status;
};

/* page block reference count scanning kernel thread's scan result status  */
typedef struct son_scand_refcount_stats {
    unsigned long idle_bpage_count;     /* not accessed base page(ACCESSED bit is not setted in pte) */
    unsigned long total_bpage_count;    /* total base page count */
    unsigned long idle_hpage_count;     /* not accessed huge page(ACCESSED bit is not setted in pmd) */
    unsigned long total_hpage_count;    /* total huge page count */    
}son_scand_stats_t;

extern spinlock_t son_scan_refcount_list_lock;
/* spinlock which is used in mm_struct list for reference count scanning kernel thread  */
extern atomic_t son_scan_pbstate_enable;	
/* atomic variable to swith on page block status scanning kernel thread(son_pbscand) 
 * usage : echo 1 > /proc/son/scan_pbstate_enable */
extern atomic_t son_debug_enable;	
/* atomic variable to swith on debugging message which can be seen in ftrace buffer 
 * usage : echo 1 > /proc/son/debug */ 
extern atomic_t son_scan_refcount_enable;
/* atomic variable to swith on reference count scanning kernel thread(son_refscand)
 * usage : echo 1 > /proc/son/scan_refcount_enable */
extern wait_queue_head_t son_scand_pbstate_wait; 
/* wait queue for page block status scanning kernel theread.(son_pbscand) */
extern wait_queue_head_t son_scand_refcount_wait;
/* wait queue for reference cout scanning kernel theread.(son_refscand) */
void son_kthread_refcount_add_entry(struct mm_struct *mm); 
/* add mm_struct to reference count scanning kernel thread's mm_struct list */
void son_kthread_refcount_del_entry(struct mm_struct *mm);
/* remove mm_struct from reference count scanning kernel thread's mm_struct list */

/* 
 * VERSION 2.
 * paper's new design of kcompactd kernel thread related variables
 */
typedef enum {                  /* page block is ... */
    SON_PB_WHITE,               /* 0%        used    */
    SON_PB_BLUE,                /* 1%  ~ 29% used    */
    SON_PB_GREEN,               /* 30% ~ 69% used    */
    SON_PB_YELLOW,              /* 70% ~ 99% used    */
    SON_PB_RED,                 /* 100%      used    */ 
    SON_PB_UMOV,   /* compaction is useless because of unmovable page */
    SON_PB_MAX                  /* max entry  */
} pb_stat_t;

/* page block utilization info node for revised kcompactd */
typedef struct son_pbutil_node_type {
    DECLARE_BITMAP(pbutil_movable_bitmap, 512);
    /* bitmap of used pages within page block range -> 64B */
    unsigned char used_movable_page;    
    /* used movable pages within page block range   -> 8B  */
    unsigned char used_unmovable_page;
    /* used unmovable pages within page block range -> 8B  */
    pb_stat_t level;
    /* calculateed page block utilization level     -> 4B  */
} pbutil_node_t; 
/* 
 * => 84B per page block 
 * e.g. 32G system ... 
 *   -> 32G(32768 MB) -> 16384 entry -> 
 *      16384 entry * 84B -> 1.3125 MB
 * FIXME 
 *  - consider changing data type of used_movable_page 
 *    and used_unmovable_page to unsigned char
 */

typedef struct son_pbutil_list_type {
    struct list_head pbutil_list_head;
    /* list for each levels in pb_stat_t */    
     spinlock_t pbutil_list_lock;
    /* spinlock which is used in page linked list for kcompactd kernel thread */  
    int cur_count;
    /* current list's pb count  */
} pbutil_list_t;

typedef struct son_pbutil_tree_type {
    struct radix_tree_root pbutil_tree_root;
    /* page status tracking radix tree  */
    spinlock_t pbutil_tree_lock;
    /* spinlock which is used in pbutil_node_t radix tree for searching in page alloc/free */
} pbutil_tree_t;

extern struct kmem_cache *son_pbutil_node_cachep;
/* slab object for page block util node which will be used in radix from pg_data_t */
extern spinlock_t son_pbutil_tree_lock;
/* spinlock which is used in pbutil_node_t radix tree for kcompactd kernel thread  */
pbutil_node_t *son_pbutil_node_alloc(void);
/* allocate pbutil_node_t to insert pb usage radix tree  */
pbutil_node_t *son_pbutil_node_delete(pbutil_tree_t *pbutil_tree, unsigned long pb_pfn_start);
/* delete pbutil_node_t from pb usage radix tree because whole page block is free state  */
pbutil_node_t *son_pbutil_node_lookup(pbutil_tree_t *pbutil_tree, unsigned long pb_pfn_start);
/* search pbutil_node_t to get pb utilization  */
int son_pbutil_node_insert(pbutil_tree_t *pbutil_tree, unsigned long pb_pfn_start, pbutil_node_t *pbutil_node);
/* insert pbutil_node_t to pb usage radix tree when pb get out of free state */
pb_stat_t calc_pbutil_level(unsigned char used_pages);
/* calculate page block utilization */

#endif
