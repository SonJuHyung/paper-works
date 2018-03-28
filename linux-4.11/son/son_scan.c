#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/bootmem.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/mm_inline.h>
#include <linux/huge_mm.h>
#include <linux/page_idle.h>
#include <linux/ksm.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/freezer.h>
#include <linux/compaction.h>
#include <linux/mmzone.h>
#include <linux/node.h>
#include <linux/workqueue.h>
#include <linux/khugepaged.h>
#include <linux/hugetlb.h>
#include <linux/migrate.h>
#include <linux/balloon_compaction.h>
#include <linux/pagevec.h>
#include <linux/random.h>
#include <asm/uaccess.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>

#include <son/son.h>
#include "../mm/internal.h"

//#define SCAND_WORKQUEUE

struct list_head son_scand_list;
spinlock_t scand_worklist_lock;
DECLARE_WAIT_QUEUE_HEAD(son_scand_wait);

#if 0
static unsigned int son_scand_sleep_millisecs = 60000; // 60s 
#endif

#ifdef SON_SCAN_WORKQUEUE

static int son_scand_kthread(void *none)
{
    return 0;
}

static int son_scand_kthread_manager(void)
{
    return 0;
}

#else

static int son_scand_check(void)
{
    if(!atomic_read(&son_scan_pbstate_enable)){        
        if(atomic_read(&son_debug_enable)){
            trace_printk("son - scan enable flag is 0 stop scanning \n"); 
        }
        return 0;
    }

    if(atomic_read(&son_debug_enable)){
        trace_printk("son - scan enable flag is 1 start scanning \n");
    }
    return 1;   
}

#if 0
static int scand_should_stop(void)
{
    return wait_event_freezable(son_scand_wait,son_scand_check()) || kthread_should_stop();
}
#endif 

static int scan_pageblock(struct son_scand_control *sc, unsigned long low_pfn,
        unsigned long end_pfn, unsigned long *index)
{
	struct page *page = NULL;
    unsigned long page_type=PB_MAXENTRY, pre_page_type=PB_MAXENTRY;
    

	for (; low_pfn < end_pfn; low_pfn++, (*index)++) {

        if(!pfn_valid_within(low_pfn))
            continue;

        page = pfn_to_page(low_pfn);

        if(!PageLRU(page)){ 
            if(PageIsolated(page)){
                page_type=PB_ISOLATE;
            }
            else{
                page_type=PB_FREE;
            }
        }else{
            page_type = get_pageblock_migratetype(page);
            switch(page_type){
                case MIGRATE_MOVABLE:
                    page_type = PB_MOVABLE;
                    break;
                case MIGRATE_UNMOVABLE:
                    page_type = PB_UNMOVABLE;
                    break;
                case MIGRATE_RECLAIMABLE:
                    page_type = PB_RECLAIMABLE;
                    break;
                case MIGRATE_HIGHATOMIC:
                    page_type = PB_HIGHATOMIC;
                    break;
            }

            if(page->son_compact_target){
                page->son_compact_target=0;
                page_type = PB_COMPACT;
            }
        }
#if 1
        if(page_type != pre_page_type){
            trace_printk("%lu,%lu \n",*index ,page_type);
        }
#endif 
#if 0
        trace_printk("%lu,%lu \n",*index ,page_type);
#endif

        sc->pb_stat[page_type]++;
        pre_page_type = page_type;

    }

    return 0;
}

#define block_start_pfn(pfn, order)	round_down(pfn, 1UL << (order))
#define block_end_pfn(pfn, order)	ALIGN((pfn) + 1, 1UL << (order))
#define pageblock_start_pfn(pfn)	block_start_pfn(pfn, pageblock_order)
// low_pfn 이 속한 page block 내의 start pfn
#define pageblock_end_pfn(pfn)		block_end_pfn(pfn, pageblock_order)
// low_pfn 이 속한 page block 내의 end pfn


static int scan_zone(struct zone *zone, struct son_scand_control *sc, unsigned long *index)
{
    unsigned long block_start_pfn, block_end_pfn, low_pfn;
	struct page *page;

    low_pfn = sc->pfn_start;
	block_start_pfn = pageblock_start_pfn(low_pfn);
    // low_pfn 이 속한 page block 내의 start pfn 

	if (block_start_pfn < low_pfn)
		block_start_pfn = low_pfn; 
        // page block 시작 위치가 zone 범위 넘어가면 재설정

	block_end_pfn = pageblock_end_pfn(low_pfn);
    // low_pfn 이 속한 page block 내의 end pfn

	for (; (block_end_pfn <= sc->pfn_end);
			low_pfn = block_end_pfn,
			block_start_pfn = block_end_pfn,
			block_end_pfn += pageblock_nr_pages){

        // free_pfn 까지 있는 모든 page block 들에 대해 page block 단위로 scan

		page = pageblock_pfn_to_page(block_start_pfn, block_end_pfn,zone);
		if (!page){
//            *index+=512;
//            trace_printk("%lu,%d \n",*index ,PB_INVALID);
            sc->pb_stat[PB_INVALID]+=512;
			continue;            
        }

        // scan 할 page block(block_start_pfn ~ block_end_pfn 범위) 내의 
        // 첫번째 struct page 가져옴 

        scan_pageblock(sc,low_pfn,block_end_pfn,index);        
	}

	return 0;
}

static int son_scand_do_work(pg_data_t *pgdat)
{
    int zoneid=0, zonemaxid=pgdat->nr_zones-1;
    struct zone *zone;
    struct son_scand_control sc = {0,};
    unsigned long index=0;

    pgdat->kscand_status=1;
	for (zoneid = 0; zoneid <= zonemaxid; zoneid++) {

        zone = &pgdat->node_zones[zoneid];
		if (!populated_zone(zone))
			continue;

        sc.pfn_start = zone->zone_start_pfn;
        sc.pfn_end = zone_end_pfn(zone);
        sc.pb_scanned = 0;
        sc.status = 1;
        
        scan_zone(zone, &sc, &index); 

        if(atomic_read(&son_debug_enable)){
            trace_printk("son - zone scan complete state : free(%lu), umov(%lu), mov(%lu), reclm(%lu), hato(%lu), iso(%lu), inv(%lu) \n", sc.pb_stat[PB_FREE], sc.pb_stat[PB_UNMOVABLE],sc.pb_stat[PB_MOVABLE],sc.pb_stat[PB_RECLAIMABLE],sc.pb_stat[PB_HIGHATOMIC],sc.pb_stat[PB_ISOLATE],sc.pb_stat[PB_INVALID]);
        }

    }

    trace_printk("son - node scan complete state : free(%lu), umov(%lu), mov(%lu), reclm(%lu), hato(%lu), iso(%lu), inv(%lu),compact(%lu) \n", sc.pb_stat[PB_FREE], sc.pb_stat[PB_UNMOVABLE],sc.pb_stat[PB_MOVABLE],sc.pb_stat[PB_RECLAIMABLE],sc.pb_stat[PB_HIGHATOMIC],sc.pb_stat[PB_ISOLATE],sc.pb_stat[PB_INVALID],sc.pb_stat[PB_COMPACT]);

    pgdat->kscand_status=0;

    return 0;
}


static int son_scand_kthread(void *p)
{
	pg_data_t *pgdat = (pg_data_t*)p;

    set_freezable();
	set_user_nice(current, MIN_NICE);

    while(!kthread_should_stop()){
        wait_event_freezable(son_scand_wait,son_scand_check());
        son_scand_do_work(pgdat);
        atomic_set(&son_scan_pbstate_enable,SON_DISABLE);
    }

    return 0;
}

static int son_scand_kthread_run(int nid)
{
	pg_data_t *pgdat = NODE_DATA(nid);

    if(pgdat->kscand)
        goto out;

    // thp_enabled
    // FIXME

    pgdat->kscand = kthread_run(son_scand_kthread, pgdat, "son_scand%d",nid); 
    // kthread_run 을 통해 내부적으로 kthread_create 호출 및 
    // create 한 kernel thread 를 바로 run queue 에 넣음 
    if(IS_ERR(pgdat->kscand)){
        pr_err("son - (err)kthread run for son_scan_kthread failed (%d)\n",nid);
        pgdat->kscand = NULL;
        goto out;
    }

    if(atomic_read(&son_debug_enable)){
        trace_printk("son - son_scan_kthread kernel thread is created \n");
    }
    // kthread_stop(son_scand_ts);
   
out:
    return 0;
}

#endif

static int __init son_scand_init(void)
{
   int nid; 
   for_each_node_state(nid,N_MEMORY) 
       son_scand_kthread_run(nid);

    return 0;
}
subsys_initcall(son_scand_init);
