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
#include <linux/sched/task.h>
#include <linux/sched/mm.h>
#include <asm/uaccess.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <linux/page_idle.h>

#include <son/son.h>
#include "../mm/internal.h"

//#define SCAND_WORKQUEUE

struct son_scand_refcount_manager {
    unsigned int mm_count;
    struct list_head son_scand_refcount_list;    
};

struct son_scand_refcount_manager son_manager;

DEFINE_SPINLOCK(son_scand_refcount_list_lock);

DECLARE_WAIT_QUEUE_HEAD(son_scand_pbstate_wait);
DECLARE_WAIT_QUEUE_HEAD(son_scand_refcount_wait);

#if 0
static unsigned int son_scand_sleep_millisecs = 60000; // 60s 
#endif

#ifdef SON_SCAN_WORKQUEUE

static int son_scand_kthread_run(void *none)
{
    return 0;
}

// FIXME 
//  - workqueue support

#else

void son_kthread_refcount_add_entry(struct mm_struct *mm)
{
	spin_lock(&son_scand_refcount_list_lock);
    son_manager.mm_count++;
	list_add_tail_rcu(&mm->son_scand_refcount_link, &son_manager.son_scand_refcount_list);
	spin_unlock(&son_scand_refcount_list_lock);
}

void son_kthread_refcount_del_entry(struct mm_struct *mm)
{
	spin_lock(&son_scand_refcount_list_lock);
    son_manager.mm_count--;    
	list_del_rcu(&mm->son_scand_refcount_link);
	spin_unlock(&son_scand_refcount_list_lock);
}


static int son_scand_pbstate_check(void)
{
    if(!atomic_read(&son_scan_pbstate_enable)){        
        if(atomic_read(&son_debug_enable)){
            trace_printk("son_pbscand - scan enable flag is 0 stop scanning \n"); 
        }
        return 0;
    }

    if(atomic_read(&son_debug_enable)){
        trace_printk("son_pbscand - scan enable flag is 1 start scanning \n");
    }
    return 1;   
}

static int scan_pageblock(struct son_scand_control *sc, unsigned long low_pfn,
        unsigned long end_pfn, unsigned long *index)
{
// verisno 1 - page usage scanning detail
#if 0
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

        if(page->son_compact_target){
            page->son_compact_target=0;
            page_type = PB_COMPACT;
        }


        if(page_type != pre_page_type)
            trace_printk("son_pbscand,%lu,%lu,%lu \n",*index,page_type,low_pfn);

        sc->pb_stat[page_type]++;
        pre_page_type = page_type;
    }
#endif

// verrsion 2 - page usage scanning just LRU and free
#if 0
	struct page *page = NULL;
    unsigned long page_type=PB_MAXENTRY, pre_page_type=PB_MAXENTRY;
	for (; low_pfn < end_pfn; low_pfn++, (*index)++) {

        if(!pfn_valid_within(low_pfn))
            continue;

        page = pfn_to_page(low_pfn);

        if(PageLRU(page)){ 
            if(PageActive(page))
                page_type=PB_ACTIVE;
            else
                page_type=PB_INACTIVE;
        }else{
            page_type=PB_FREE;
        }

        if(page->son_compact_target){
            page->son_compact_target=0;
        }

        if(page_type != pre_page_type)
            trace_printk("son_pbscand,%lu,%lu,%lu \n",*index,page_type,low_pfn);

        sc->pb_stat[page_type]++;
        pre_page_type = page_type;

    }
#endif 

// verrsion 3 - page block scanning by used percentage
#if 1
	struct page *page = NULL;
    int used_pages=0, used_percentage=0;

	for (; low_pfn < end_pfn; low_pfn++) {

        if(!pfn_valid_within(low_pfn))
            continue;

        page = pfn_to_page(low_pfn);
        if(PageLRU(page)){ 
            used_pages++;
        }
    }
#endif
    (*index)++;  
    used_percentage=used_pages*10/512;
    trace_printk("son_pbscand,%lu,%lu,%d \n",*index,low_pfn,used_percentage);

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

static int son_scand_pbstate_do_work(pg_data_t *pgdat)
{
    int zoneid=0, zonemaxid=pgdat->nr_zones-1;
    struct zone *zone;
    struct son_scand_control sc = {0,};
    unsigned long index=0;

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
//            trace_printk("son_pbscand - zone scan complete state : free(%lu), umov(%lu), mov(%lu), reclm(%lu), hato(%lu), iso(%lu), inv(%lu) \n", sc.pb_stat[PB_FREE], sc.pb_stat[PB_UNMOVABLE],sc.pb_stat[PB_MOVABLE],sc.pb_stat[PB_RECLAIMABLE],sc.pb_stat[PB_HIGHATOMIC],sc.pb_stat[PB_ISOLATE],sc.pb_stat[PB_INVALID]);
            trace_printk("son_pbscand_comple,free(%lu),inactive(%lu),active(%lu) \n", sc.pb_stat[PB_FREE], sc.pb_stat[PB_INACTIVE],sc.pb_stat[PB_ACTIVE]);
        }

    }

//    trace_printk("son_pbscand_comple,free(%lu), umov(%lu), mov(%lu), reclm(%lu), hato(%lu), iso(%lu), inv(%lu),compact(%lu) \n", sc.pb_stat[PB_FREE], sc.pb_stat[PB_UNMOVABLE],sc.pb_stat[PB_MOVABLE],sc.pb_stat[PB_RECLAIMABLE],sc.pb_stat[PB_HIGHATOMIC],sc.pb_stat[PB_ISOLATE],sc.pb_stat[PB_INVALID],sc.pb_stat[PB_COMPACT]);
    trace_printk("son_pbscand_comple,%lu,%lu,%lu \n", sc.pb_stat[PB_FREE], sc.pb_stat[PB_INACTIVE],sc.pb_stat[PB_ACTIVE]);

    return 0;
}

static int son_kthread_pbstate(void *p)
{
	pg_data_t *pgdat = (pg_data_t*)p;

    set_freezable();
	set_user_nice(current, MIN_NICE);

    while(!kthread_should_stop()){
        wait_event_freezable(son_scand_pbstate_wait,son_scand_pbstate_check());
        son_scand_pbstate_do_work(pgdat);
        atomic_set(&son_scan_pbstate_enable,SON_DISABLE);
    }

    return 0;
}


static int son_scand_refcount_check(void)
{
    if(!atomic_read(&son_scan_refcount_enable)){        
        if(atomic_read(&son_debug_enable)){
            trace_printk("son_refscand - scan enable flag is 0 stop scanning \n"); 
        }
        return 0;
    }

    if(atomic_read(&son_debug_enable)){
        trace_printk("son_refscand - refcount scan enable flag is 1 start scanning \n");
    }
    return 1;   
}

static int son_pte_walker(pte_t *pte, unsigned long addr,
		unsigned long next, struct mm_walk *walk)
{
    struct mm_struct *mm=NULL;
	struct son_scand_refcount_stats *walker_stats;
	struct page *page=NULL;
    page_utilmap_t *utilmap;
	unsigned long pfn;
	int ret=0, frequency=0;

	mm = walk->mm;
	walker_stats = &mm->son_mm_scand_stats;

    if (pte && !pte_none(*pte)) 
        page = pte_page(*pte);
        // pte_t 즉 page table entry 가 비어있지 않다면 
        // pte 의 pfn number 통해 struct page 계산

    if (page && !PageTransCompound(page)) {
		walker_stats->total_bpage_count++;
        // base page count 증가
		//pfn = (pte_val(*pte) & PTE_PFN_MASK) >> PAGE_SHIFT;
        pfn = pte_pfn(*pte);
        // pte 에 해당하는 physical page의 pfn구함  
		page = page_idle_get_page(pfn);
        // LRU list 에 있는 page 인지 검사 후 get_page
		if (page) {
            // LRU 에 있는 page 로 정상적으로 idle page tracking 처리 되었다면
			page_idle_clear_pte_refs(page);
            // page 에 해당되는 pte 에 ACCESSED bit 설정된 경우.. 
            //  - ACCESSED bit clear - 다음번 scan 검사에 활용
            //  - idle flag clear - idle page가 아니므로
            //  - youg flag set - page reclaim 에 사용을 위해

            utilmap = &page->page_util_info;	
            // page 당 struct utilmap_node 를 가지고 있으며 
            // 이 struct 를 통해 해당 page 의 접근 빈도 및 
            // huge page 영역 내의 할당 상태를 관리
			utilmap->page = page;

			if (!utilmap)
				goto out;

			bitmap_shift_right(utilmap->freq_bitmap, utilmap->freq_bitmap, 1, FREQ_BITMAP_SIZE);
            // 이번차례의 접근여부를 bitmap의 MSB에 기록하기 위해 
            // freq_bitmap 에서 FREQ_BITMAP_SIZE 만큼의 bit 를 가져와 
            // 1 bit 만큼 shift 연산 수행             
            //
            //    1110 1111 
            // -> 0111 0111 

			if (page_is_idle(page)) {
                // PG_idle 이 남아있다면 ACCESSED bit 가 설정되지 않은 상황으로
                // page가 접근되지 않은 것이므로..
				walker_stats->idle_bpage_count++;
                // idle page count증가해주고
				bitmap_clear(utilmap->freq_bitmap, FREQ_BITMAP_SIZE-1, 1); 
                // bitmap의 MSB를 0으로 설정
                //
                //     0111 0111
                //     ^ 
                //     |
                //     0 로 clear 
                //  => 0111 0111
			} else {
                // PG_idle 이 clear되었다면 ACCESSED bit 가 설정된 상황으로
                // page가 접근된 것이므로..
				bitmap_set(utilmap->freq_bitmap, FREQ_BITMAP_SIZE-1, 1);
                // 
                //     0111 0111 
                //     ^ 
                //     |
                //     1 로 set
                //  => 1111 0111
			    frequency=bitmap_weight(utilmap->freq_bitmap,FREQ_BITMAP_SIZE);
//                trace_printk("son_refscand,bp_pfn:%lu,freq:%d \n",pfn,frequency);
                trace_printk("son_refscand,bp,%lu,%d \n",pfn,frequency);
			}

			set_page_idle(page);
			put_page(page);
		}
    }
out:
    return ret;

}

static int son_pmd_walker(pmd_t *pmd, unsigned long addr,
		unsigned long next, struct mm_walk *walk)
{
    struct mm_struct *mm=NULL;
	struct son_scand_refcount_stats *walker_stats;
	struct page *page=NULL;
    page_utilmap_t *utilmap;
	pte_t *pte;
	unsigned long _addr, pfn;
	int ret=0, frequency=0;

	mm = walk->mm;
	walker_stats = &mm->son_mm_scand_stats;

	if (pmd_trans_huge(*pmd)) {
        // THP 인 경우
		walker_stats->total_hpage_count++;

		//pfn = (pmd_val(*pmd) & PTE_PFN_MASK) >> PAGE_SHIFT;        
        pfn = pmd_pfn(*pmd);
        // pmd 에 해당하는 entry 에 적힌 pfn 계산
		page = page_idle_get_page(pfn);
        // LRU list 에 있는 page 인지 검사 후 get_page
		if (page) {
			page_idle_clear_pte_refs(page);
            // THP 인 경우, pmd 에 해당되는 entry 자체가 2MB 크기 하나의 page 임 
            // page 에 해당되는 pte 에 accessed bit 설정된 경우.. 
            //  - ACCESSED bit clear - 다음번 scan 검사에 활용
            //  - idle flag clear - idle page가 아니므로
            //  - youg flag set - page reclaim 에 사용을 위해

			VM_BUG_ON(!PageCompound(page));
            utilmap = &page->page_util_info;	
            // page 당 struct utilmap_node 를 가지고 있으며 
            // 이 struct 를 통해 해당 page 의 접근 빈도 및 
            // huge page 영역 내의 할당 상태를 관리
			utilmap->page = page;

			if (!utilmap)
				goto out;

			bitmap_shift_right(utilmap->freq_bitmap, utilmap->freq_bitmap, 1, FREQ_BITMAP_SIZE);
            // 이번차례의 접근여부를 bitmap의 MSB에 기록하기 위해 
            // freq_bitmap 에서 FREQ_BITMAP_SIZE 만큼의 bit 를 가져와 
            // 1 bit 만큼 shift 연산 수행             
            //
            //    1110 1111 
            // -> 0111 0111 

			if (page_is_idle(page)) {
                // PG_idle 이 남아있다면 ACCESSED bit 가 설정되지 않은 상황으로
                // page가 접근되지 않은 것이므로..
				walker_stats->idle_hpage_count++;
                // idle page count증가해주고
				bitmap_clear(utilmap->freq_bitmap, FREQ_BITMAP_SIZE-1, 1); 
                // bitmap의 MSB를 0으로 설정
                //
                //     0111 0111
                //     ^ 
                //     |
                //     0 로 clear 
                //  => 0111 0111 
			}
			else { 
                // PG_idle 이 clear되었다면 ACCESSED bit 가 설정된 상황으로
                // page가 접근된 것이므로..
				bitmap_set(utilmap->freq_bitmap, FREQ_BITMAP_SIZE-1, 1);
                // 
                //     0111 0111 
                //     ^ 
                //     |
                //     1 로 set
                //  => 1111 0111
			    frequency=bitmap_weight(utilmap->freq_bitmap,FREQ_BITMAP_SIZE);
//                trace_printk("son_refscand,hp_pfn:%lu,freq:%d \n",pfn,frequency);
                trace_printk("son_refscand,hp,%lu,%d \n",pfn,frequency);
            }

			set_page_idle(page);
			put_page(page);
		}    
    }else{
        // THP 가 아닐 경우
        pte = pte_offset_map(pmd, addr);
        // page table 을 찾기 위해 pte entry 값 가져옴
        _addr = addr;
        // 주소 백업
        for (;;) {
            // page table 내의 512 개 page table entry 순회 
            // _addr ~ next 즉 _addr ~ _addr + 2MB
            son_pte_walker(pte, _addr, _addr + PAGE_SIZE, walk);

            _addr += PAGE_SIZE;
            // page table
            if (_addr == next)
                break;
            pte++;
        }
    }
out:
    return ret;
}

static int son_scand_refcount_do_walk(struct mm_struct *mm) 
{
	struct vm_area_struct *vma = NULL;
	struct mm_walk son_walker = {
		.pmd_entry = son_pmd_walker,
		.mm = mm,
	};
	int err = 0;

    vma = mm->mmap;

    for ( ;vma != NULL; vma = vma->vm_next) {
		
		err = walk_page_vma(vma, &son_walker);

		if (err) {
			trace_printk("son_refscand - error in vma walk\n");
			return err;
		}

//		cond_resched();
	}

    return 0;
}

static int son_scand_refcount_do_work(pg_data_t *pgdat)
{
	struct mm_struct *mm;
	struct task_struct *tsk;
    struct son_scand_refcount_stats *walker_node_stats;
    struct son_scand_refcount_stats *walker_mm_stats;

	int err;

    walker_node_stats = &pgdat->son_node_scand_stats;
    walker_node_stats->total_hpage_count=0;
    walker_node_stats->total_bpage_count=0;
    walker_node_stats->idle_hpage_count=0;
    walker_node_stats->idle_bpage_count=0;


    trace_printk("son_refscand,start,-1,-1\n");
    // Scanning per-application anonymous pages
	list_for_each_entry_rcu(mm, &son_manager.son_scand_refcount_list, son_scand_refcount_link) {

		if (!mm) 
			continue;

		if (atomic_read(&mm->mm_users) == 0)
			continue;

		rcu_read_lock();
		tsk = rcu_dereference(mm->owner);

        if (!tsk) 
            goto unlock_exit;

		if (atomic_read(&(tsk)->usage) == 0)
			goto unlock_exit;

		get_task_struct(tsk);
		mm = get_task_mm(tsk);
		rcu_read_unlock();

		VM_BUG_ON(!mm);

//		memset(&walker_stats, 0, sizeof(struct son_scand_refcount_stats));
        walker_mm_stats = &mm->son_mm_scand_stats;
        walker_mm_stats->total_hpage_count=0;
        walker_mm_stats->total_bpage_count=0;
        walker_mm_stats->idle_hpage_count=0;
        walker_mm_stats->idle_bpage_count=0;

        err = son_scand_refcount_do_walk(mm);
        // mm scanning 시작

        walker_node_stats->total_hpage_count+=walker_mm_stats->total_hpage_count;
        walker_node_stats->total_bpage_count+=walker_mm_stats->total_bpage_count;
        walker_node_stats->idle_hpage_count+=walker_mm_stats->idle_hpage_count;
        walker_node_stats->idle_bpage_count+=walker_mm_stats->idle_bpage_count;

        mmput(mm);
        put_task_struct(tsk);
    }

    trace_printk("son_refscand,end,-1,-1,%lu,%lu,%lu,%lu \n",walker_node_stats->total_hpage_count,walker_node_stats->idle_hpage_count,walker_node_stats->total_bpage_count,walker_node_stats->idle_bpage_count);
    return 0;

unlock_exit:
	rcu_read_unlock();
	return 0;

}

static int son_kthread_refcount(void *p)
{
	pg_data_t *pgdat = (pg_data_t*)p;

    set_freezable();
	set_user_nice(current, MIN_NICE);

    son_manager.mm_count=0;
	INIT_LIST_HEAD(&son_manager.son_scand_refcount_list);

    while(!kthread_should_stop()){
        wait_event_freezable(son_scand_refcount_wait,son_scand_refcount_check());
        son_scand_refcount_do_work(pgdat);
        atomic_set(&son_scan_refcount_enable,SON_DISABLE);
    }

    return 0;
}


static int son_scand_kthread_run(int nid)
{
	pg_data_t *pgdat = NODE_DATA(nid);

    // thp_enabled
    // FIXME

    if(pgdat->kscand_pbstate)
        goto out;
    
    pgdat->kscand_pbstate = kthread_run(son_kthread_pbstate, pgdat, "son_pbscand%d",nid); 
    // kthread_run 을 통해 내부적으로 kthread_create 호출 및 
    // create 한 kernel thread 를 바로 run queue 에 넣음 
    if(IS_ERR(pgdat->kscand_pbstate)){
        pr_err("son - (err)kthread run for son_kthread_pbstate failed (%d)\n",nid);
        pgdat->kscand_pbstate = NULL;
        goto out;
    }

    if(atomic_read(&son_debug_enable)){
        trace_printk("son - son_kthread_pbstate kernel thread is created \n");
    }
  
    if(pgdat->kscand_refcount)
        goto out;

    pgdat->kscand_refcount = kthread_run(son_kthread_refcount, pgdat, "son_refscand%d",nid); 
    // kthread_run 을 통해 내부적으로 kthread_create 호출 및 
    // create 한 kernel thread 를 바로 run queue 에 넣음 
    if(IS_ERR(pgdat->kscand_refcount)){
        pr_err("son - (err)kthread run for son_kthread_refcount failed (%d)\n",nid);
        pgdat->kscand_refcount = NULL;
        goto out;
    }

    if(atomic_read(&son_debug_enable)){
        trace_printk("son - son_kthread_refcount kernel thread is created \n");
    }


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
