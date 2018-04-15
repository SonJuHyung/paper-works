#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/vmstat.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/vmstat.h>
#include <linux/compaction.h>
#include <linux/math64.h>
#include <linux/mmzone.h>
#include <linux/mm_types.h>
#include <linux/nodemask.h>

#define TARGET_PID 2649

#if 0
//static bool son_kcompactd_node_suitable(pg_data_t *pgdat)
//{
//	int zoneid;
//	struct zone *zone;
//	enum zone_type classzone_idx = pgdat->kcompactd_classzone_idx;
//
//	for (zoneid = 0; zoneid <= classzone_idx; zoneid++) {
//		zone = &pgdat->node_zones[zoneid];
//
//		if (!populated_zone(zone))
//			continue;
//
//		if (compaction_suitable(zone, pgdat->kcompactd_max_order, 0,
//					classzone_idx) == COMPACT_CONTINUE)
//			return true;
//	}
//
//	return false;
//}

//void son_wakeup_kcompactd(pg_data_t *pgdat, int order, int classzone_idx)
//{
//    int ret1=-1, ret2=-1;
//
//	ret1=waitqueue_active(&pgdat->kcompactd_wait);
//    if(ret1 != 0)
//        ret2 = son_kcompactd_node_suitable(pgdat);
//
//    trace_printk("son_kprobe,%d,%d,%d,%d\n", pgdat->kcompactd_max_order,order,ret1,ret2);

//    unsigned long free_pages1=0,free_pages2=0;
//    free_pages1 = node_page_state(pgdat->node_id, NR_FREE_PAGES);
//    free_pages2 = global_page_state(NR_FREE_PAGES);
//    trace_printk("%lu,%lu \n",free_pages1, free_pages2);
//	long mark,free_pages,reserve;
//    int freepage_wmark_state,frag_state,o,i;
//    int kctd_max_order=pgdat->kcompactd_max_order;
//
//    for (i = 0; i <= classzone_idx; i++) {
//        struct zone *z = pgdat->node_zones + i;
//        mark = (long)high_wmark_pages(z);
//        free_pages = zone_page_state(z, NR_FREE_PAGES);
//        reserve=frag_state=o=0;
//        freepage_wmark_state=1;
//
//
//        if (z->percpu_drift_mark && free_pages < z->percpu_drift_mark)
//            free_pages = zone_page_state_snapshot(z, NR_FREE_PAGES);
//
//        free_pages -= (1 << order) - 1;
//        reserve=z->lowmem_reserve[classzone_idx];
//
//        // 1) need to ckeck if kcompactd is woken up by wmark value.
//        if (free_pages <= mark + z->lowmem_reserve[classzone_idx]){
//            freepage_wmark_state=0;
//        }
//
//        // 2) need to check if kcompactd is woken up by ext frag state
//        for (o = order; o < MAX_ORDER; o++) {
//            struct free_area *area = &z->free_area[o];
//            int mt;
//
//            if (!area->nr_free)
//                continue;
//
//            for (mt = 0; mt < MIGRATE_PCPTYPES; mt++) {
//                if (!list_empty(&area->free_list[mt])){
//                    frag_state=1; 
//                    break;
//                }
//            }
//        }
//        trace_printk("son-%d zone, frp:%ld, wh:%ld, rsv:%ld, odr:%d, czi:%d -> fws:%d, frgs:%d, kmo:%d\n", i,free_pages, mark,reserve,order,classzone_idx,freepage_wmark_state,frag_state,kctd_max_order);
//    }


	/* Always end with a call to jprobe_return(). */
//	jprobe_return();
//	return 0;
//}

#if 0
static void son_kswapd_try_to_sleep(pg_data_t *pgdat, int alloc_order, int reclaim_order,
				unsigned int classzone_idx)
{
    trace_printk("son_kprobe,%d,%d\n", pgdat->kcompactd_max_order,alloc_order);

    jprobe_return();
}

#endif

#endif

#if 0



static void unusable_show_print(struct seq_file *m,
					pg_data_t *pgdat, struct zone *zone)
{
	unsigned int order;
	int index;
	struct contig_page_info info;

	seq_printf(m, "Node %d, zone %8s ",
				pgdat->node_id,
				zone->name);
	for (order = 0; order < MAX_ORDER; ++order) {
		fill_contig_page_info(zone, order, &info);
		index = unusable_free_index(order, &info);
		seq_printf(m, "%d.%03d ", index / 1000, index % 1000);
	}

	seq_putc(m, '\n');
}
#endif

struct son_contig_page_info {
	unsigned long free_pages;
	unsigned long free_blocks_total;
	unsigned long free_pages_suitable;
	unsigned long free_blocks_suitable;
};

static void fill_contig_page_info(struct zone *zone,
				unsigned int suitable_order,
				struct son_contig_page_info *info)
{
	unsigned int order;

	info->free_pages = 0;
	info->free_blocks_total = 0;
	info->free_blocks_suitable = 0;

	for (order = 0; order < MAX_ORDER; order++) {
		unsigned long blocks;

		/* Count number of free blocks */
		blocks = zone->free_area[order].nr_free;
		info->free_blocks_total += blocks;

		/* Count free base pages */
		info->free_pages += blocks << order;

		/* Count the suitable free blocks */
		if (order >= suitable_order)
			info->free_blocks_suitable += blocks <<
						(order - suitable_order);
	}
}

static int unusable_free_index(unsigned int order,
				struct son_contig_page_info *info)
{
	/* No free memory is interpreted as all free memory is unusable */

	if (info->free_pages == 0)
		return 1000;

	/*
	 * Index should be a value between 0 and 1. Return a value to 3
	 * decimal places.
	 *
	 * 0 => no fragmentation
	 * 1 => high fragmentation
	 */
	return div_u64((info->free_pages - (info->free_blocks_suitable << order)) * 1000ULL, info->free_pages);
}

//const char* workload = "redis-server";
const char* workload = "mongod";
//const char* workload = "java";

struct page *
son__alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order,
			struct zonelist *zonelist, nodemask_t *nodemask)
{
	struct task_struct *tsk_cur = current;
    int pid_cur=0,nid=0;
    
    struct son_contig_page_info info;
    pg_data_t *pgdat=NULL;
    struct zone *zone;
    unsigned long free_pages=-1, free_pages_raw=-1;
    int ufi_raw = -1, target_order=9;

	/* check memoryless node */

    if(tsk_cur){
        if(!strcmp(tsk_cur->comm,workload)){
            pid_cur=tsk_cur->pid;
            pgdat=NODE_DATA(nid);
            zone=&pgdat->node_zones[ZONE_NORMAL];
            free_pages_raw=global_page_state(NR_FREE_PAGES);
            free_pages=free_pages_raw<<(PAGE_SHIFT-10);
            fill_contig_page_info(zone,target_order,&info);
            ufi_raw = unusable_free_index(target_order,&info);

            trace_printk("%d,%s,%d,%lu,%d.%03d \n", pid_cur, tsk_cur->comm, order, free_pages, ufi_raw/1000,ufi_raw%1000);
        }
    }

    jprobe_return();
}

static struct jprobe son_jprobe = {
//	.entry			= son_wakeup_kcompactd,
	.entry			= son__alloc_pages_nodemask,
// 	.entry          = son_kswapd_try_to_sleep,
	.kp = {
//		.symbol_name	= "wakeup_kcompactd",
		.symbol_name	= "__alloc_pages_nodemask",
//		.symbol_name	= "kswapd_try_to_sleep",

	},
};

static int __init jprobe_init(void)
{
	int ret;

    ret = register_jprobe(&son_jprobe);
	if (ret < 0) {
		pr_err("son - jprobe register failed, returned %d\n", ret);
		return -1;
	}
	pr_info("son - planted jprobe at %p, handler addr %p\n",
	       son_jprobe.kp.addr, son_jprobe.entry);
	return 0;
}

static void __exit jprobe_exit(void)
{
	unregister_jprobe(&son_jprobe);
	pr_info("jprobe at %p unregistered\n", son_jprobe.kp.addr);
}

module_init(jprobe_init)
module_exit(jprobe_exit)
MODULE_LICENSE("GPL");
