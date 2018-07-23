#ifndef _SON_H_
#define _SON_H_

//#include <linux/mm.h>

#define SON_SUCCESS             0
#define SON_ENABLE              1
#define SON_DISABLE             0

#define SON_PBSCAND_ENABLE      1
#define SON_REFSCAND_ENABLE     0
#define SON_DEBUG_ENABLE        0
#define SON_PBSTAT_ENABLE       1


/* 
 * VERSION 1. Monitoring phase
 * paper's monitoring kernel thread related variables
 */
#if SON_PBSCAND_ENABLE 

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

extern atomic_t son_scan_pbstate_enable;	
/* atomic variable to swith on page block status scanning kernel thread(son_pbscand) 
 * usage : echo 1 > /proc/son/scan_pbstate_enable */

extern wait_queue_head_t son_scand_pbstate_wait; 
/* wait queue for page block status scanning kernel theread.(son_pbscand) */
#endif

#if SON_DEBUG_ENABLE
extern atomic_t son_debug_enable;	
/* atomic variable to swith on debugging message which can be seen in ftrace buffer 
 * usage : echo 1 > /proc/son/debug */ 
#endif

#if SON_REFSCAND_ENABLE

/* page block reference count scanning kernel thread's scan result status  */
typedef struct son_scand_refcount_stats {
    unsigned long idle_bpage_count;     /* not accessed base page(ACCESSED bit is not setted in pte) */
    unsigned long total_bpage_count;    /* total base page count */
    unsigned long idle_hpage_count;     /* not accessed huge page(ACCESSED bit is not setted in pmd) */
    unsigned long total_hpage_count;    /* total huge page count */    
}son_scand_stats_t;


extern atomic_t son_scan_refcount_enable;
/* atomic variable to swith on reference count scanning kernel thread(son_refscand)
 * usage : echo 1 > /proc/son/scan_refcount_enable */

extern spinlock_t son_scan_refcount_list_lock;
/* spinlock which is used in mm_struct list for reference count scanning kernel thread  */

extern wait_queue_head_t son_scand_refcount_wait;
/* wait queue for reference cout scanning kernel theread.(son_refscand) */

void son_kthread_refcount_add_entry(struct mm_struct *mm); 
/* add mm_struct to reference count scanning kernel thread's mm_struct list */
void son_kthread_refcount_del_entry(struct mm_struct *mm);
/* remove mm_struct from reference count scanning kernel thread's mm_struct list */

#endif

/* 
 * VERSION 2. New Design phase
 * paper's new design of kcompactd kernel thread related variables
 */
#if SON_PBSTAT_ENABLE

#include <linux/radix-tree.h>

/* return value of functinos  */
#define SON_PBSTAT_SUCCESS                  SON_SUCCESS
#define SON_PBSTAT_ERR_BUDDY                -1
#define SON_PBSTAT_ERR_NOT_BUDDY            -2
#define SON_PBSTAT_ERR_MEMALLOC             -3
#define SON_PBSTAT_ERR_NODE_NOT_PRESENT     -4

/* bitmap size in pbutil_node_t */
#define PBUTIL_BMAP_SIZE        512
#define SON_COMP_PBBATCH_MIG    64
#define SON_COMP_PBBATCH_FRE    64

/* compactino mode  */
typedef enum {
    SON_COMPMODE_ORIGIN,
    SON_COMPMODE_REVISD,
    SON_COMPMODE_ORIGIN_MONITOR,
    SON_COMPMODE_MAX
} comp_mode_t;

typedef enum {                  /* page block is ... */
    SON_PB_WHITE,               /* 0%        used    */
    SON_PB_BLUE,                /* 1%  ~ 29% used    */
    SON_PB_SYSFS_MIN = SON_PB_BLUE,
    SON_PB_GREEN,               /* 30% ~ 69% used    */
    SON_PB_YELLOW,              /* 70% ~ 99% used    */ 
    SON_PB_ORANGE,
    SON_PB_SYSFS_MAX = SON_PB_ORANGE,
    SON_PB_RED,                 /* 100%      used    */ 
    SON_PB_UMOV,   /* compaction is useless because of unmovable page */
    SON_PB_ISOMG,    /* isolated pb during compaction */
    SON_PB_ISOFR,
    SON_PB_MAX                  /* max entry  */
} pb_stat_t;

/* name of each page block utilization state  */
extern char * const pbstat_names[SON_PB_MAX];
/* atomic page block utilization level threshold value
 * which will be used when isolating migrate pages */
extern atomic_t son_pbstat_comp_mig_level;
/* atomic page block utilization level threshold value
 * which will be used when isolating free pages */
extern atomic_t son_pbstat_comp_free_level;
/* sysfs compaction mode (0:original 1:revised) */
extern atomic_t son_pbstat_comp_mode;
extern atomic_t son_pbstat_mig_threshold;
extern atomic_t compact_run;

/* page allocatino type which will be logged in page->mgtype */
typedef enum {                  
    SON_PB_UNMOVABLE,           
    SON_PB_MOVABLE,             
    SON_PB_RECLAIMABLE,
    SON_PB_BUDDY,               
    SON_PB_ISOLATE,
} pg_stat_t;

/* page block utilization info node for revised kcompactd */
typedef struct son_pbutil_node_type {
    DECLARE_BITMAP(pbutil_movable_bitmap, PBUTIL_BMAP_SIZE);
    /* bitmap of used pages within page block range -> 64B */
    unsigned long used_movable_page;    
    /* used movable pages within page block range   -> 8B  */
    unsigned long used_unmovable_page;
    /* used unmovable pages within page block range -> 8B  */
    unsigned long isolated_movable_pages;
    /* isolated page by compaction                  -> 8B  */
    unsigned long pb_head_pfn;
    /* head page frame number of page block 
     * which is used as key in radix tree           -> 8B  */
    pb_stat_t level;
    /* calculateed page block utilization level     -> 4B  */
    struct list_head pbutil_level;
} pbutil_node_t; 

/* 
 * => 100B per page block 
 * e.g. 32G system ... 
 *   -> 32G(32768 MB) -> 16384 entry -> 
 *      16384 entry * 100 B -> 1638400 B -> 1.5625 MB 
 * FIXME 
 *  - consider changing data type of used_movable_page 
 *    and used_unmovable_page to unsigned char
 */

#if 0
/* version 2  */
typedef struct son_pbutil_list_type {
    struct list_head pbutil_list_head;
    /* list for each levels in pb_stat_t */    
    spinlock_t pbutil_list_lock;
    /* spinlock which is used in page linked list for kcompactd kernel thread */  
    int cur_count;
    /* current list's pb count  */
} pbutil_list_t;
#endif
/* version 1 */
/* linked list per each page block level 
 * which contains head page's page frame 
 * number information */
typedef struct son_pbutil_list_type {
    struct list_head pbutil_list_head;
    /* list for each levels in pb_stat_t */    
    int cur_count;
    /* current list's pb count  */
} pbutil_list_t;

/* page block utilization radix tree 
 * to search corresponding node */
typedef struct son_pbutil_tree_type {
    struct radix_tree_root pbutil_tree_root;
    /* page status tracking radix tree  */
    spinlock_t pbutil_tree_lock;
    /* spinlock which is used in pbutil_node_t radix tree for searching in page alloc/free */
    int node_count;
    /* for debugging purpose  */
    unsigned long mig_success;
    unsigned long mig_fail;
} pbutil_tree_t;

extern struct kmem_cache *son_pbutil_node_cachep;
/* slab object for page block util node which will be used in radix from pg_data_t */
extern spinlock_t son_pbutil_tree_lock;
/* spinlock which is used in pbutil_node_t radix tree for kcompactd kernel thread  */
pbutil_node_t *son_pbutil_node_alloc(unsigned long block_start_pfn);
/* allocate pbutil_node_t to insert pb usage radix tree  */
void son_pbutil_node_free(pbutil_node_t *node);
/* free pbutil_node_t   */
pbutil_node_t *son_pbutil_node_delete(pbutil_tree_t *pbutil_tree, unsigned long pb_pfn_start);
/* delete pbutil_node_t from pb usage radix tree because whole page block is free state  */
pbutil_node_t *son_pbutil_node_lookup(pbutil_tree_t *pbutil_tree, unsigned long pb_pfn_start);
/* search pbutil_node_t to get pb utilization  */
int son_pbutil_node_insert(pbutil_tree_t *pbutil_tree, unsigned long pb_pfn_start, pbutil_node_t *pbutil_node);
/* insert pbutil_node_t to pb usage radix tree when pb get out of free state */
pb_stat_t calc_pbutil_level(unsigned long used_pages);
/* calculate page block utilization */

/*
 * TODO
 *  - make son_pbutil_update_alloc,son_pbutil_update_free only handles only 
 *    MIGRATE_MOVABLE pages.
 *  - implement son_pbutil_update_alloc_umov, son_pbutil_update_free_umov 
 *    functions which handles only unmovable pages. 
 *  - fix spin_lock recursive problem in _umov functinos.
 */
int son_pbutil_update_alloc(struct page *page, unsigned int order);
/* update page block utilization status in page allocating phase  */
int son_pbutil_update_free(struct page *page, unsigned int order);
/* update page block utilization status in page freeing phase  */

int son_pbutil_update_alloc_umov(struct page *page, unsigned int order);
/* FIXME  */
int son_pbutil_update_free_umov(struct page *page, unsigned int order);
/* FIXME  */ 
size_t bitmap_print(unsigned long *bitmap, int nbits,char *buf, size_t size);
size_t son_bitmap_scnprintf(unsigned long *bitmap, int nbits,char *buf, size_t size);

#endif

#endif
