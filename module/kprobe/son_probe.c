#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/mmzone.h>
#include <linux/vmstat.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/types.h>

#define TARGET_PID 2649

//const char* workload = "redis-server";
//const char* workload = "mongod";
const char* workload = "java";

struct page *
son__alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order,
			struct zonelist *zonelist, nodemask_t *nodemask)
{
	struct task_struct *tsk_cur = current;
    int pid_cur=0;
#if 0
    pid_cur=tsk_cur->pid;
    struct task_struct *tsk_par = current->parent;
    int pid_par=tsk_par->pid;
#endif
#if 1
    if(tsk_cur){
        if(!strcmp(tsk_cur->comm,workload)){
            pid_cur=tsk_cur->pid;
            trace_printk("pid_cur:%d, cur_name:%s, order:%d \n", pid_cur, tsk_cur->comm, order);
        }
    }
#endif        
#if 0
    if(!strcmp(tsk_cur->comm,workload))
        trace_printk("pid_cur:%d, cur_name:%s, pid_par:%d, par_name:%s, order:%d \n",pid_cur,tsk_cur->comm,pid_par,tsk_par->comm,order);
#endif
    jprobe_return();
}

void son_wakeup_kcompactd(pg_data_t *pgdat, int order, int classzone_idx)
{
	long mark,free_pages,reserve;
    int freepage_wmark_state,frag_state,o,i;
    int kctd_max_order=pgdat->kcompactd_max_order;

    for (i = 0; i <= classzone_idx; i++) {
        struct zone *z = pgdat->node_zones + i;
        mark = (long)high_wmark_pages(z);
        free_pages = zone_page_state(z, NR_FREE_PAGES);
        reserve=frag_state=o=0;
        freepage_wmark_state=1;


        if (z->percpu_drift_mark && free_pages < z->percpu_drift_mark)
            free_pages = zone_page_state_snapshot(z, NR_FREE_PAGES);

        free_pages -= (1 << order) - 1;
        reserve=z->lowmem_reserve[classzone_idx];

        // 1) need to ckeck if kcompactd is woken up by wmark value.
        if (free_pages <= mark + z->lowmem_reserve[classzone_idx]){
            freepage_wmark_state=0;
        }

        // 2) need to check if kcompactd is woken up by ext frag state
        for (o = order; o < MAX_ORDER; o++) {
            struct free_area *area = &z->free_area[o];
            int mt;

            if (!area->nr_free)
                continue;

            for (mt = 0; mt < MIGRATE_PCPTYPES; mt++) {
                if (!list_empty(&area->free_list[mt])){
                    frag_state=1; 
                    break;
                }
            }
        }
        trace_printk("son-%d zone, frp:%ld, wh:%ld, rsv:%ld, odr:%d, czi:%d -> fws:%d, frgs:%d, kmo:%d\n", i,free_pages, mark,reserve,order,classzone_idx,freepage_wmark_state,frag_state,kctd_max_order);
    }


	/* Always end with a call to jprobe_return(). */
	jprobe_return();
//	return 0;
}

static struct jprobe son_jprobe = {
//	.entry			= son_wakeup_kcompactd,
	.entry			= son__alloc_pages_nodemask,
	.kp = {
//		.symbol_name	= "wakeup_kcompactd",
		.symbol_name	= "__alloc_pages_nodemask",

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
