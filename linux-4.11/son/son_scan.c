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
#include <linux/jiffies.h>

#include <son/son.h>
#include "../mm/internal.h"


#if SON_PBSCAND_ENABLE
/* if page block scanning option is enabled compile it  */

DECLARE_WAIT_QUEUE_HEAD(son_scand_pbstate_wait);


static int son_scand_pbstate_check(void)
{
    if(!atomic_read(&son_scan_pbstate_enable)){       
        return 0;
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
    if(used_pages!=0){
        used_percentage=used_pages*10/512;
        if(used_percentage == 0)
            used_percentage=1;
    }else{
        used_percentage=0;
    }
    trace_printk("son_pbscand,%lu,%lu,%d \n",*index,low_pfn,used_percentage);

    return 0;
}
#if 0
#define block_start_pfn(pfn, order)	round_down(pfn, 1UL << (order))
#define block_end_pfn(pfn, order)	ALIGN((pfn) + 1, 1UL << (order))
#define pageblock_start_pfn(pfn)	block_start_pfn(pfn, pageblock_order)
/* start page frame number within page block */
#define pageblock_end_pfn(pfn)		block_end_pfn(pfn, pageblock_order)
/* end page frame number within page block */
#endif
/* INFO - move above functions to internal.h  */

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
    }

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

#endif
/* endof SON_PBSCAND_ENABLE  */

#if SON_REFSCAND_ENABLE 
/* if page reference count scanning option is enabled compile it  */

struct son_scand_refcount_manager {
    unsigned int mm_count;
    struct list_head son_scand_refcount_list;    
};

struct son_scand_refcount_manager son_manager;

DEFINE_SPINLOCK(son_scand_refcount_list_lock);
DECLARE_WAIT_QUEUE_HEAD(son_scand_refcount_wait);
static unsigned int son_scand_refcount_sleep_millisecs = 6000;

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


static int son_scand_refcount_check(void)
{
    if(!atomic_read(&son_scan_refcount_enable)){        
        return 0;
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

            utilmap = &page->page_util_ref_info;	
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
//                trace_printk("son_refscand,bp,%lu,%d \n",pfn,frequency);
//                #
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
            utilmap = &page->page_util_ref_info;	
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
//                trace_printk("son_refscand,hp,%lu,%d \n",pfn,frequency);
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
    return 0;

unlock_exit:
	rcu_read_unlock();
	return 0;

}

static int son_scand_refcount_has_work(void)
{
	return son_scand_refcount_sleep_millisecs && 
        (!list_empty(&son_manager.son_scand_refcount_list));
}

static int son_scand_refcount_wait_event(void)
{
	return son_scand_refcount_sleep_millisecs && 
        (!list_empty(&son_manager.son_scand_refcount_list) || kthread_should_stop());
}

static void son_scand_refcount_wait_work(void)
{
    if(son_scand_refcount_has_work())
		wait_event_freezable_timeout(son_scand_refcount_wait,0,msecs_to_jiffies(son_scand_refcount_sleep_millisecs));
    else
        wait_event_freezable(son_scand_refcount_wait,son_scand_refcount_wait_event());
}
#define REFSCAND_SLEEP_PERIODICAL 0
static int son_kthread_refcount(void *p)
{
	pg_data_t *pgdat = (pg_data_t*)p;

    set_freezable();
	set_user_nice(current, MAX_NICE);

    son_manager.mm_count=0;

    while(!kthread_should_stop()){
#if REFSCAND_SLEEP_PERIODICAL
        son_scand_refcount_do_work(pgdat);
        son_scand_refcount_wait_work();
#else
        wait_event_freezable(son_scand_refcount_wait,son_scand_refcount_check());
        son_scand_refcount_do_work(pgdat);
        atomic_set(&son_scan_refcount_enable,SON_DISABLE);
#endif
    }

    return 0;
}

#endif
/* end of SON_REFSCAND_ENABLE  */

static int son_scand_kthread_run(int nid)
{
	pg_data_t *pgdat = NODE_DATA(nid);

    // thp_enabled
    // FIXME
#if SON_PBSCAND_ENABLE 
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

#endif

#if SON_REFSCAND_ENABLE
    if(pgdat->kscand_refcount)
        goto out;

	INIT_LIST_HEAD(&son_manager.son_scand_refcount_list);

    pgdat->kscand_refcount = kthread_run(son_kthread_refcount, pgdat, "son_refscand%d",nid); 
    // kthread_run 을 통해 내부적으로 kthread_create 호출 및 
    // create 한 kernel thread 를 바로 run queue 에 넣음 
    if(IS_ERR(pgdat->kscand_refcount)){
        pr_err("son - (err)kthread run for son_kthread_refcount failed (%d)\n",nid);
        pgdat->kscand_refcount = NULL;
        goto out;
    }

    if(!list_empty(&son_manager.son_scand_refcount_list))
        wake_up_interruptible(&son_scand_refcount_wait);
 
#endif
out:
    return 0;
}

#if SON_PBSTAT_ENABLE 

pb_stat_t calc_pbutil_level(unsigned long used_pages)
{
    pb_stat_t ret = SON_PB_RED;
    unsigned long used_percentage;

    if(used_pages!=0){
        used_percentage=used_pages*10/512;
        if(used_percentage == 0)
            used_percentage=1;
    }else{
        used_percentage=0;
    }

    switch(used_percentage){
            /* page  block is... */
        case 0 :
            /* 0%        used    */
            ret = SON_PB_WHITE;
            break;
        case 1:
        case 2:
            /* 1% ~ 29%  used    */
            ret = SON_PB_BLUE;
            break;
        case 3:
        case 4:
        case 5:
            /* 30% ~ 59% used    */
            ret = SON_PB_GREEN;            
            break;
        case 6:
        case 7:
        case 8:
        case 9:           
            /* 60% ~ 99% used    */
            ret = SON_PB_YELLOW;
            break;
        case 10:
            /* 100%      used  */
            ret = SON_PB_RED;
            break;
        default:
            ret = SON_PB_RED;
            break;
    }
    return ret;
}

pbutil_node_t *son_pbutil_node_delete(pbutil_tree_t *pbutil_tree, unsigned long pb_pfn_start)
{
	return radix_tree_delete(&pbutil_tree->pbutil_tree_root, pb_pfn_start);
}

pbutil_node_t *son_pbutil_node_lookup(pbutil_tree_t *pbutil_tree, unsigned long pb_pfn_start)
{
	return radix_tree_lookup(&pbutil_tree->pbutil_tree_root, pb_pfn_start);
}

int son_pbutil_node_insert(pbutil_tree_t *pbutil_tree, unsigned long pb_pfn_start, pbutil_node_t *pbutil_node)
{
	return radix_tree_insert(&pbutil_tree->pbutil_tree_root, pb_pfn_start, pbutil_node);
}
#if 0
static void son_pbstat_add_entry(struct zone *zone, struct page *page, int level)
{    
    pbutil_list_t *pbutil_list = NULL;

    spin_lock(&zone->pbutil_list_lock);
    pbutil_list = &zone->son_pbutil_list[level];
    list_add_tail(&page->pbutil_level,&pbutil_list->pbutil_list_head);
    trace_printk("list added\n");
    pbutil_list->cur_count++;
    spin_unlock(&zone->pbutil_list_lock);
}

static void son_pbstat_del_entry(struct zone *zone, struct page *page, int level)
{
    pbutil_list_t *pbutil_list = NULL;

    spin_lock(&zone->pbutil_list_lock);
    pbutil_list = &zone->son_pbutil_list[level];
    list_del(&page->pbutil_level);
    pbutil_list->cur_count--;
    spin_unlock(&zone->pbutil_list_lock);
}
#endif

int son_pbutil_update_alloc(struct page *page, unsigned int order)
{
    struct zone *zone = NULL;
    pg_data_t *pbutil_pgdat = NULL;
    pbutil_tree_t *pbutil_tree = NULL;
    pbutil_node_t *pbutil_node = NULL;
    pbutil_list_t *pbutil_list = NULL, *pbutil_list_pre = NULL;   
    unsigned long pb_start_pfn, pb_end_pfn, pfn,pfn_end;
    unsigned long low_pfn, low_end_pfn;
    pb_stat_t cur_level = SON_PB_WHITE, pre_level = SON_PB_WHITE;
    int mgtype, ret = SON_PBSTAT_SUCCESS;
    
    /* Step 1. Initialize vatiables.*/         
    zone = page_zone(page);
    spin_lock(&zone->pbutil_list_lock);

    mgtype = page->mgtype;
    pbutil_pgdat = zone->zone_pgdat;
    pbutil_tree = &pbutil_pgdat->son_pbutil_tree;

    pfn = page_to_pfn(page);          
    pfn_end = pfn + (1UL << order);
    pb_start_pfn = block_start_pfn(pfn, pageblock_order); 
    pb_end_pfn = block_end_pfn(pfn, pageblock_order);

    /* 
     * Example Cases.
     * e.g. if page block is composed of 8 pages ...
     * case 1. single pb range
     *      first loop) - search pfn(24) pb in radix tree
     *       pb_start_pfn(24)    pb_end_pfn(32)
     *            |                 |
     *            *                 *
     *  ... 0 0 | 0 0 0 0 1 1 1 1 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 ...
     *                    ^         ^
     *                    |         |
     *               pfn(28)   pfn_end(32)
     *           low_pfn(28)  low_end_pfn(32)
     *
     * case 2. twro pb range
     *      first loop) - search pfn(24) pb in radix tree
     *                          low_end_pfn(32)
     *      pb_start_pfn(24)     pb_end_pfn(32)
     *            |                 |
     *            *                 *
     *  ... 0 0 | 0 0 0 0 1 1 1 1 | 1 1 1 1 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 ...
     *                    ^                 ^
     *                    |                 |
     *                pfn(28)          pfn_end(36)
     *               low_pfn(28)
     *
     *      second loop) - search pfn(32) pb in radix tree
     *                           low_pfn(32)
     *                        pb_start_pfn(32)   pb_end_pfn(40)
     *                              |                 |
     *                              *                 *
     *  ... 0 0 | 0 0 0 0 1 1 1 1 | 1 1 1 1 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 ...
     *                    ^                 ^
     *                    |                 |
     *                pfn(28)            pfn_end(36)
     *                                 low_end_pfn(36)
     *
     * case 3. three pb range (order > pageblock_order)
     *      first loop)
     *                          low_end_pfn(32)
     *      pb_start_pfn(24)     pb_end_pfn(32)
     *            |                 |
     *            *                 *
     *  ... 0 0 | 0 0 0 0 1 1 1 1 | 1 1 1 1 1 1 1 1 | 1 1 1 1 0 0 0 0 | 0 0 ...
     *                    ^                                   ^
     *                    |                                   |
     *                   pfn(28)                          pfn_end(44)
     *                low_pfn(28) 
     *
     *      second loop)
     *                          low_pfn(32)         low_end_pfn(40)
     *                          pb_start_pfn(32)     pb_end_pfn(40)
     *                              |                 |
     *                              *                 *
     *  ... 0 0 | 0 0 0 0 1 1 1 1 | 1 1 1 1 1 1 1 1 | 1 1 1 1 0 0 0 0 | 0 0 ...
     *                    ^                                   ^
     *                    |                                   |
     *                   pfn(28)                          pfn_end(44)
     *
     *      third loop)
     *                                            low_pfn(40)       
     *                                          pb_start_pfn(40)     pb_end_pfn(48)
     *                                                |                 |
     *                                                *                 *
     *  ... 0 0 | 0 0 0 0 1 1 1 1 | 1 1 1 1 1 1 1 1 | 1 1 1 1 0 0 0 0 | 0 0 ...
     *                    ^                                   ^
     *                    |                                   |
     *                   pfn(28)                          pfn_end(44)
     *                                                  low_end_pfn(44)
     */

    for(low_pfn = pb_start_pfn, low_end_pfn = pb_end_pfn; 
            pb_start_pfn < pfn_end ; 
            pb_start_pfn = pb_end_pfn,
            pb_end_pfn += pageblock_nr_pages,
            low_pfn = pb_start_pfn,
            low_end_pfn = pb_end_pfn){

        if(pfn > pb_start_pfn){
            low_pfn = pfn;
        }

        if(pfn_end < pb_end_pfn){
            low_end_pfn = pfn_end;
        }

        /* Step 2. Search pbutil_node_t in radix tree  */ 
//        rcu_read_lock();
        pbutil_node = son_pbutil_node_lookup(pbutil_tree, pb_start_pfn);
//        rcu_read_unlock();
        if(!pbutil_node){
            /* ok. this page is the only allocated page within page. 
             * we need to allocate new pbutil_node_t and insert to 
             * pbutil_tree_t and add the page to pbutil_list_t */

            pbutil_node = son_pbutil_node_alloc();
            if(pbutil_node){ 

#if SON_DEBUG_ENABLE
                trace_printk("pbutil_node is allocated in page alloc process \n");                
#endif                 
                /* Step 3. Update pbutil_node's bitmap and page count */ 
                if(mgtype == SON_PB_MOVABLE){

                    /* if current allocation is movable allocation 
                     * log status to bitmap  
                     * 
                     * set (low_end_pfn - low_pfn ) bits starting from low_pfn 
                     *                     pb k    pb k+1   pb k+2   low_end_pfn - low_pfn
                     *  case 1.          00000000 00000000 00000000 
                     *    first  loop -> 00001111 00000000 00000000     4
                     *
                     *  case 2.          00000000 00000000 00000000 
                     *    first  loop -> 00001111 00000000 00000000     4
                     *    second loop -> 00001111 11110000 00000000     4
                     *
                     *  case 3.          00000000 00000000 00000000 
                     *    first  loop -> 00001111 00000000 00000000     4
                     *    second loop -> 00001111 11111111 00000000     8
                     *    third  loop -> 00001111 11111111 11110000     4
                     */

                    bitmap_set(pbutil_node->pbutil_movable_bitmap, 
                            low_pfn - pb_start_pfn, low_end_pfn - low_pfn);

                    pbutil_node->used_movable_page += (low_end_pfn - low_pfn);
                    cur_level = calc_pbutil_level(pbutil_node->used_movable_page);
                    pbutil_node->pb_head_pfn = pb_start_pfn;
                    INIT_LIST_HEAD(&pbutil_node->pbutil_level);
                }else{

                    /* if current allocation is unmovable allocation 
                     * just increases unmovable count and doesn't 
                     * log allocation status to bitmap */
                    pbutil_node->used_unmovable_page += (low_end_pfn - low_pfn);
                    cur_level = SON_PB_UMOV;
                    pbutil_node->pb_head_pfn = pb_start_pfn;
                    INIT_LIST_HEAD(&pbutil_node->pbutil_level);
                }

                /* 
                 * Step 4. insert newly create pbutil_node_t to 
                 *         page block utilization radix tree 
                 *         unmovable page doesn't have bitmap info.
                 */
                pbutil_node->level = cur_level;
 //               spin_lock(&pbutil_tree->pbutil_tree_lock);
                son_pbutil_node_insert(pbutil_tree, pb_start_pfn, pbutil_node);
                pbutil_tree->node_count++;
//                spin_unlock(&pbutil_tree->pbutil_tree_lock); 

                /* 
                 * Step 5. insert page to calculated level linked list  
                 *         SON_PB_BLUE or SON_PB_UMOV
                 */ 
#if 0
                /* version 2 */                
                pbutil_list = &zone->son_pbutil_list[cur_level];
                spin_lock(&pbutil_list->pbutil_list_lock);
                list_add_tail(&pbutil_node->pbutil_level,&pbutil_list->pbutil_list_head);
                pbutil_list->cur_count++;
                spin_unlock(&pbutil_list->pbutil_list_lock);
#endif
                /* version 1 */                
 //               spin_lock(&zone->pbutil_list_lock);
                pbutil_list = &zone->son_pbutil_list[cur_level];
                list_add_tail(&pbutil_node->pbutil_level,&pbutil_list->pbutil_list_head);
                pbutil_list->cur_count++;
//                spin_unlock(&zone->pbutil_list_lock);
#if SON_DEBUG_ENABLE
                trace_printk("alloc new, success,pb(%lu ~ %lu),pg(%lu ~ %lu),order(%d/%lu/%lu),pbstat_names(%d) \n",pb_start_pfn, pb_end_pfn, pfn, pfn_end, order, pbutil_node->used_movable_page,pbutil_node->used_unmovable_page,cur_level);
#endif
            }else{
                ret = SON_PBSTAT_ERR_MEMALLOC;
                goto out;
            }
        }else{
#if SON_DEBUG_ENABLE
            trace_printk("pbutil_node is found in allocating process \n");                
#endif
            /* ok. this some pages within page block is already allocated. 
             * we need to update page block status and check whether pb 
             * status level is changed. if level is changed, delete page 
             * from previous level's linked list and insert page into 
             * current level's linked list, */

            pre_level = pbutil_node->level; 
            /* Step 3. Update pbutil_node's bitmap and page count */ 
            if(pre_level == SON_PB_ISOMG || pre_level == SON_PB_ISOFR){
                if(mgtype == SON_PB_MOVABLE){
                    bitmap_set(pbutil_node->pbutil_movable_bitmap, 
                            low_pfn - pb_start_pfn, low_end_pfn - low_pfn);
                    pbutil_node->used_movable_page += (low_end_pfn - low_pfn);
                }else{
                    pbutil_node->used_unmovable_page += (low_end_pfn - low_pfn);
                }            
                goto out;
            }else{
                if(mgtype == SON_PB_MOVABLE){
                    /* if current allocation is movable allocation 
                     * log status to bitmap  */

                    bitmap_set(pbutil_node->pbutil_movable_bitmap, 
                            low_pfn - pb_start_pfn, low_end_pfn - low_pfn);

                    pbutil_node->used_movable_page += (low_end_pfn - low_pfn);
                    if(pre_level == SON_PB_UMOV)
                        cur_level = pre_level;
                    else
                        cur_level = calc_pbutil_level(pbutil_node->used_movable_page);
                    /* if previous level is SON_PB_UMOV, type of level 
                     * can not be changed until all unmovable page freed  
                     * so keep previous level */

                }else{
                    /* if current allocation is unmovable allocation 
                     * just increases unmovable count and doesn't 
                     * log allocation status to bitmap */
                    pbutil_node->used_unmovable_page += (low_end_pfn - low_pfn);
                    cur_level = SON_PB_UMOV; 
                }
            }

            if(pre_level != cur_level){
                /* if previous level is SON_PB_UMOV, can not reach here  
                 * only consider level transition between movable page block*/

                /* 
                 * Step 4. delete page from previous level linked list and
                 *         insert page into current level linked list.
                 */                 
#if 0 
                /* version 2 */
                pbutil_list_pre = &zone->son_pbutil_list[pre_level];
                spin_lock(&pbutil_list_pre->pbutil_list_lock);
                list_del(&pbutil_node->pbutil_level);
                pbutil_list_pre->cur_count--;
                spin_unlock(&pbutil_list_pre->pbutil_list_lock);

                pbutil_list = &zone->son_pbutil_list[cur_level];
                spin_lock(&pbutil_list->pbutil_list_lock);
                list_add_tail(&pbutil_node->pbutil_level,&pbutil_list->pbutil_list_head);
                pbutil_list->cur_count++;
                spin_unlock(&pbutil_list->pbutil_list_lock);

                pbutil_node->level = cur_level;
#endif 
                /* version 1 */
//                spin_lock(&zone->pbutil_list_lock);
                pbutil_list_pre = &zone->son_pbutil_list[pre_level];
                list_del(&pbutil_node->pbutil_level);
                pbutil_list_pre->cur_count--;

                pbutil_list = &zone->son_pbutil_list[cur_level];
                list_add_tail(&pbutil_node->pbutil_level,&pbutil_list->pbutil_list_head);
                pbutil_list->cur_count++;
                pbutil_node->level = cur_level;
//                spin_unlock(&zone->pbutil_list_lock);
            }
#if SON_DEBUG_ENABLE
            trace_printk("alloc found, success,pb(%lu ~ %lu),pg(%lu ~ %lu),order(%d/%lu/%lu),pbstat_names(%d) \n",pb_start_pfn, pb_end_pfn, pfn, pfn_end, order, pbutil_node->used_movable_page,pbutil_node->used_unmovable_page,cur_level);
#endif

        }
    }
out:
    if(ret != SON_PBSTAT_SUCCESS)
        trace_printk("alloc, error(%d), pb(%lu ~ %lu), pg(%lu ~ %lu), order(%d) \n",ret,pb_start_pfn, pb_end_pfn, pfn, pfn_end, order);
    spin_unlock(&zone->pbutil_list_lock);

    return ret;
}

int son_pbutil_update_free(struct page *page, unsigned int order)
{
    struct zone *zone = NULL;
    pg_data_t *pbutil_pgdat = NULL;
    pbutil_tree_t *pbutil_tree = NULL;
    pbutil_node_t *pbutil_node = NULL;
    pbutil_list_t *pbutil_list = NULL, *pbutil_list_pre = NULL;   
    unsigned long pb_start_pfn, pb_end_pfn, pfn,pfn_end;
    unsigned long low_pfn, low_end_pfn;
    pb_stat_t cur_level = SON_PB_WHITE, pre_level = SON_PB_WHITE;
    int mgtype, ret = SON_PBSTAT_SUCCESS;
    /* Step 1. Initialize vatiables.*/
    zone = page_zone(page);
    spin_lock(&zone->pbutil_list_lock);

    mgtype = page->mgtype;
    pbutil_pgdat = zone->zone_pgdat;
    pbutil_tree = &pbutil_pgdat->son_pbutil_tree;

    pfn = page_to_pfn(page);          
    pfn_end = pfn + (1UL << order);
    pb_start_pfn = block_start_pfn(pfn, pageblock_order); 
    pb_end_pfn = block_end_pfn(pfn, pageblock_order);

    for(low_pfn = pb_start_pfn, low_end_pfn = pb_end_pfn; 
            pb_start_pfn < pfn_end ; 
            pb_start_pfn = pb_end_pfn,
            pb_end_pfn += pageblock_nr_pages,
            low_pfn = pb_start_pfn,
            low_end_pfn = pb_end_pfn){

        if(pfn > pb_start_pfn){
            low_pfn = pfn;
        }

        if(pfn_end < pb_end_pfn){
            low_end_pfn = pfn_end;
        }

        /* Step 2. Search pbutil_node_t in radix tree  */ 
//        rcu_read_lock();
        pbutil_node = son_pbutil_node_lookup(pbutil_tree, pb_start_pfn);
//        rcu_read_unlock();
        if(!pbutil_node){
            /* if page are already allocated  but doesn't have page block 
             * utilization info. ackward situation. recheck status and 
             * insert information to the radix tree. */
#if SON_DEBUG_ENABLE
            trace_printk("pbutil_node is not found in freeing process \n");                
#endif             
            ret = SON_PBSTAT_ERR_NODE_NOT_PRESENT;
            goto out;
        }else{
            /* ok. pbutil_node is found. we need to update bitmap information 
             * and allocation status.  */

#if SON_DEBUG_ENABLE
            trace_printk("pbutil_node is found in allocating process \n");                
#endif
            pre_level = pbutil_node->level; 
            if(pre_level == SON_PB_ISOMG || pre_level == SON_PB_ISOFR){
               
                if(mgtype == SON_PB_BUDDY){
                    trace_printk("putback_isofr-used:%lu/iso:%lu (%10s)\n",
                            pbutil_node->used_movable_page,
                            pbutil_node->isolated_movable_pages,
                            pbstat_names[pre_level]);

                    if(pbutil_node->isolated_movable_pages > 0){
                        pbutil_node->isolated_movable_pages--;
                        if(pbutil_node->isolated_movable_pages > 0)
                            goto out;                    
                        else
                            cur_level = calc_pbutil_level(pbutil_node->used_movable_page);
                    }else if(!pbutil_node->isolated_movable_pages){
                        cur_level = calc_pbutil_level(pbutil_node->used_movable_page); 
                    }else{
                        trace_printk("error pbutil_node's isolated_movable_pages is negative number \n");
                        goto out;
                    }                                          
                       
                }else{
                    if(mgtype == SON_PB_MOVABLE){
                        bitmap_clear(pbutil_node->pbutil_movable_bitmap, 
                                low_pfn - pb_start_pfn, low_end_pfn - low_pfn);
                        pbutil_node->used_movable_page -= (low_end_pfn - low_pfn);
                    } else {                
                        pbutil_node->used_unmovable_page -= (low_end_pfn - low_pfn);
                    }
                    goto out;     
                }        
            }else{ 
                /* Step 3. Update pbutil_node's bitmap and page count */ 
                if(mgtype == SON_PB_MOVABLE){

                    /* if current allocation is movable allocation 
                     * log status to bitmap  */
                    bitmap_clear(pbutil_node->pbutil_movable_bitmap, 
                            low_pfn - pb_start_pfn, low_end_pfn - low_pfn);
                    pbutil_node->used_movable_page -= (low_end_pfn - low_pfn);
                    if(pre_level == SON_PB_UMOV)
                        cur_level = pre_level;
                    else
                        cur_level = calc_pbutil_level(pbutil_node->used_movable_page);
                    /* if previous level is SON_PB_UMOV, type of level 
                     * can not be changed until all unmovable page freed  
                     * we area freeing movable page so keep previous level */
                } else {
                    /* if current allocation is unmovable allocation 
                     * just increases unmovable count and doesn't 
                     * log allocation status to bitmap */
                    pbutil_node->used_unmovable_page -= (low_end_pfn - low_pfn);

                    if(pbutil_node->used_unmovable_page <= 0)
                        cur_level = calc_pbutil_level(pbutil_node->used_movable_page);
                    else
                        cur_level = pre_level;
                    /* all the unmovable pages are freed we can allocate 
                     * THP in this page block by compaction!! */
                }
            }

            if(pre_level != cur_level){
                /* if previous level is SON_PB_UMOV, can not reach here  
                 * only consider level transition between movable page block*/

                /* 
                 * Step 4. delete page from previous level linked list and
                 *         insert page into current level linked list.
                 */
#if 0
                /* version 2 */
                pbutil_list_pre = &zone->son_pbutil_list[pre_level];
                spin_lock(&pbutil_list_pre->pbutil_list_lock);
                list_del(&pbutil_node->pbutil_level);
                pbutil_list_pre->cur_count--;
                spin_unlock(&pbutil_list_pre->pbutil_list_lock);

                pbutil_list = &zone->son_pbutil_list[cur_level];
                spin_lock(&pbutil_list->pbutil_list_lock);
                list_add_tail(&pbutil_node->pbutil_level,&pbutil_list->pbutil_list_head);
                pbutil_list->cur_count++;
                spin_unlock(&pbutil_list->pbutil_list_lock);

                pbutil_node->level = cur_level;
#endif 
                /* version 1 */
                //                spin_lock(&zone->pbutil_list_lock);
                pbutil_list_pre = &zone->son_pbutil_list[pre_level];
                list_del(&pbutil_node->pbutil_level);
                pbutil_list_pre->cur_count--;

                pbutil_list = &zone->son_pbutil_list[cur_level];
                list_add_tail(&pbutil_node->pbutil_level,&pbutil_list->pbutil_list_head);
                pbutil_list->cur_count++;
                pbutil_node->level = cur_level;
                //                spin_unlock(&zone->pbutil_list_lock); 
                if(pre_level == SON_PB_ISOFR && mgtype == SON_PB_BUDDY){
                    trace_printk("putback_isofr-used:%lu/iso:%lu (%10s:%d -> %10s:%d)\n", 
                            pbutil_node->used_movable_page,
                            pbutil_node->isolated_movable_pages,
                            pbstat_names[pre_level], 
                            pbutil_list_pre->cur_count, 
                            pbstat_names[cur_level], 
                            pbutil_list->cur_count);
                }

                
            }                      
#if SON_DEBUG_ENABLE
            trace_printk("free found, success,pb(%lu ~ %lu),pg(%lu ~ %lu),order(%d/%lu/%lu),pbstat_names(%d) \n",pb_start_pfn, pb_end_pfn, pfn, pfn_end, order, pbutil_node->used_movable_page,pbutil_node->used_unmovable_page,cur_level);
#endif

#if 0
            /*
             * Step 6. update radix tree.
             */
            if(!pbutil_node->used_unmovable_page && !pbutil_node->used_movable_page){
                spin_lock(&pbutil_tree->pbutil_tree_lock);
                son_pbutil_node_delete(pbutil_tree, pb_start_pfn);                
                son_pbutil_node_free(pbutil_node);
                spin_unlock(&pbutil_tree->pbutil_tree_lock);
            }   

#endif
        }        
    }
out:
    if(ret != SON_PBSTAT_SUCCESS)
        trace_printk("free, error(%d), pb(%lu ~ %lu), pg(%lu ~ %lu), order(%d) \n",ret,pb_start_pfn, pb_end_pfn, pfn, pfn_end, order);
    spin_unlock(&zone->pbutil_list_lock);

    return ret;
}

int son_pbutil_update_alloc_umov(struct page *page, unsigned int order)
{
    struct zone *zone = NULL;
    pg_data_t *pbutil_pgdat = NULL;
    pbutil_tree_t *pbutil_tree = NULL;
    pbutil_node_t *pbutil_node = NULL;
    pbutil_list_t *pbutil_list = NULL, *pbutil_list_pre = NULL;   
    unsigned long pb_start_pfn, pb_end_pfn, pfn,pfn_end;
    unsigned long low_pfn, low_end_pfn;
    pb_stat_t cur_level = SON_PB_WHITE, pre_level = SON_PB_WHITE;
    int mgtype, ret = SON_PBSTAT_SUCCESS;
    
    zone = page_zone(page);
    spin_lock(&zone->pbutil_list_lock);
   
    mgtype = page->mgtype;
    pbutil_pgdat = zone->zone_pgdat;
    pbutil_tree = &pbutil_pgdat->son_pbutil_tree;

    pfn = page_to_pfn(page);          
    pfn_end = pfn + (1UL << order);
    pb_start_pfn = block_start_pfn(pfn, pageblock_order); 
    pb_end_pfn = block_end_pfn(pfn, pageblock_order);

    for(low_pfn = pb_start_pfn, low_end_pfn = pb_end_pfn; 
            pb_start_pfn < pfn_end ; 
            pb_start_pfn = pb_end_pfn,
            pb_end_pfn += pageblock_nr_pages,
            low_pfn = pb_start_pfn,
            low_end_pfn = pb_end_pfn){

        if(pfn > pb_start_pfn){
            low_pfn = pfn;
        }

        if(pfn_end < pb_end_pfn){
            low_end_pfn = pfn_end;
        }

//        rcu_read_lock();
        pbutil_node = son_pbutil_node_lookup(pbutil_tree, pb_start_pfn);
//        rcu_read_unlock();
        if(!pbutil_node){
//            pbutil_node = son_pbutil_node_alloc();
            trace_printk("SJH_alloc_new umovable page is allocated \n");
#if 0
            if(pbutil_node){ 

                if(mgtype == SON_PB_MOVABLE){

                    bitmap_set(pbutil_node->pbutil_movable_bitmap, 
                            low_pfn - pb_start_pfn, low_end_pfn - low_pfn);
                    pbutil_node->used_movable_page += (low_end_pfn - low_pfn);
                    cur_level = calc_pbutil_level(pbutil_node->used_movable_page);
                    pbutil_node->pb_head_pfn = pb_start_pfn;
                    INIT_LIST_HEAD(&pbutil_node->pbutil_level);
                }else{

                    pbutil_node->used_unmovable_page += (low_end_pfn - low_pfn);
                    cur_level = SON_PB_UMOV;
                    pbutil_node->pb_head_pfn = pb_start_pfn;
                    INIT_LIST_HEAD(&pbutil_node->pbutil_level);
                }

                pbutil_node->level = cur_level;
                spin_lock(&pbutil_tree->pbutil_tree_lock);
                son_pbutil_node_insert(pbutil_tree, pb_start_pfn, pbutil_node);
                pbutil_tree->node_count++;
                spin_unlock(&pbutil_tree->pbutil_tree_lock); 

                spin_lock(&zone->pbutil_list_lock);
                pbutil_list = &zone->son_pbutil_list[cur_level];
                list_add_tail(&pbutil_node->pbutil_level,&pbutil_list->pbutil_list_head);
                pbutil_list->cur_count++;
                spin_unlock(&zone->pbutil_list_lock);

            }else{
                ret = SON_PBSTAT_ERR_MEMALLOC;
                goto out;
            }
#endif
        }else{
            trace_printk("SJH_alloc_found umovable page is allocated \n");

#if 0
            if(mgtype == SON_PB_MOVABLE){

                bitmap_set(pbutil_node->pbutil_movable_bitmap, 
                        low_pfn - pb_start_pfn, low_end_pfn - low_pfn);
                pbutil_node->used_movable_page += (low_end_pfn - low_pfn);
                pre_level = pbutil_node->level; 
                if(pre_level == SON_PB_UMOV)
                    cur_level = pre_level;
                else
                    cur_level = calc_pbutil_level(pbutil_node->used_movable_page);
            }else{

                pbutil_node->used_unmovable_page += (low_end_pfn - low_pfn);
                pre_level = pbutil_node->level;
                cur_level = SON_PB_UMOV; 
            }

            if(pre_level != cur_level){

                spin_lock(&zone->pbutil_list_lock);
                pbutil_list_pre = &zone->son_pbutil_list[pre_level];
                list_del(&pbutil_node->pbutil_level);
                pbutil_list_pre->cur_count--;

                pbutil_list = &zone->son_pbutil_list[cur_level];
                list_add_tail(&pbutil_node->pbutil_level,&pbutil_list->pbutil_list_head);
                pbutil_list->cur_count++;
                pbutil_node->level = cur_level;
                spin_unlock(&zone->pbutil_list_lock);
            }
#endif
        }
    }
out:
    spin_unlock(&zone->pbutil_list_lock);

    return ret;
}

int son_pbutil_update_free_umov(struct page *page, unsigned int order)
{
    struct zone *zone = NULL;
    pg_data_t *pbutil_pgdat = NULL;
    pbutil_tree_t *pbutil_tree = NULL;
    pbutil_node_t *pbutil_node = NULL;
    pbutil_list_t *pbutil_list = NULL, *pbutil_list_pre = NULL;   
    unsigned long pb_start_pfn, pb_end_pfn, pfn,pfn_end;
    unsigned long low_pfn, low_end_pfn;
    pb_stat_t cur_level = SON_PB_WHITE, pre_level = SON_PB_WHITE;
    int mgtype, ret = SON_PBSTAT_SUCCESS;

    zone = page_zone(page);
    spin_lock(&zone->pbutil_list_lock);

    mgtype = page->mgtype;
    pbutil_pgdat = zone->zone_pgdat;
    pbutil_tree = &pbutil_pgdat->son_pbutil_tree;

    pfn = page_to_pfn(page);          
    pfn_end = pfn + (1UL << order);
    pb_start_pfn = block_start_pfn(pfn, pageblock_order); 
    pb_end_pfn = block_end_pfn(pfn, pageblock_order);

    for(low_pfn = pb_start_pfn, low_end_pfn = pb_end_pfn; 
            pb_start_pfn < pfn_end ; 
            pb_start_pfn = pb_end_pfn,
            pb_end_pfn += pageblock_nr_pages,
            low_pfn = pb_start_pfn,
            low_end_pfn = pb_end_pfn){

        if(pfn > pb_start_pfn){
            low_pfn = pfn;
        }

        if(pfn_end < pb_end_pfn){
            low_end_pfn = pfn_end;
        }

//        rcu_read_lock();
        pbutil_node = son_pbutil_node_lookup(pbutil_tree, pb_start_pfn);
//        rcu_read_unlock();
        if(!pbutil_node){
            trace_printk("SJH_free_notfound can't find node \n");
  
            ret = SON_PBSTAT_ERR_NODE_NOT_PRESENT;
            goto out;
        }else{
            trace_printk("SJH_free_found can't find node \n");

#if 0 
            if(mgtype == SON_PB_MOVABLE){

                bitmap_clear(pbutil_node->pbutil_movable_bitmap, 
                        low_pfn - pb_start_pfn, low_end_pfn - low_pfn);
                pbutil_node->used_movable_page -= (low_end_pfn - low_pfn);
                pre_level = pbutil_node->level; 
                if(pre_level == SON_PB_UMOV)
                    cur_level = pre_level;
                else
                    cur_level = calc_pbutil_level(pbutil_node->used_movable_page);

            }else{

                pbutil_node->used_unmovable_page -= (low_end_pfn - low_pfn);
                pre_level = pbutil_node->level;

                if(pbutil_node->used_unmovable_page <= 0)
                    cur_level = calc_pbutil_level(pbutil_node->used_movable_page);
                else
                    cur_level = pre_level;
            }

            if(pre_level != cur_level){

                spin_lock(&zone->pbutil_list_lock);
                pbutil_list_pre = &zone->son_pbutil_list[pre_level];
                list_del(&pbutil_node->pbutil_level);
                pbutil_list_pre->cur_count--;

                pbutil_list = &zone->son_pbutil_list[cur_level];
                list_add_tail(&pbutil_node->pbutil_level,&pbutil_list->pbutil_list_head);
                pbutil_list->cur_count++;
                pbutil_node->level = cur_level;
                spin_unlock(&zone->pbutil_list_lock);
            }                      
#endif
        } 
    }
out:
    spin_unlock(&zone->pbutil_list_lock);

    return ret;
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
