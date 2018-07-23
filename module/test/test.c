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
#include <linux/bitmap.h>
#include <son/son.h>
#if 0
#define BMAP_SIZE 512

size_t bitmap_scnprintf(unsigned long *bitmap, int nbits,
			char *buf, size_t size)
{
	/* current bit is 'cur', most recently seen range is [rbot, rtop] */
	int cur, rbot, rtop;
	bool first = true;
	size_t ret = 0;

	rbot = cur = find_first_bit(bitmap, nbits);
	while (cur < nbits) {
		rtop = cur;
		cur = find_next_bit(bitmap, nbits, cur + 1);
		if (cur < nbits && cur <= rtop + 1)
			continue;

		if (!first)
			ret += scnprintf(buf + ret, size - ret, ",");

		first = false;

		ret += scnprintf(buf + ret, size - ret, "%d", rbot);
		if (rbot < rtop)
			ret += scnprintf(buf + ret, size - ret, "-%d", rtop);

		rbot = cur;
	}
	return ret;
}

size_t bitmap_print(unsigned long *bitmap, int nbits,
			char *buf, size_t size)
{
	/* current bit is 'cur', most recently seen range is [rbot, rtop] */
	int cur, rbot, rtop,i,rbot_pre=0;
	size_t ret = 0;

	rbot = cur = find_first_bit(bitmap, nbits);
	while (cur < nbits) {
		rtop = cur;
		cur = find_next_bit(bitmap, nbits, cur + 1);
		if (cur < nbits && cur <= rtop + 1){            
			continue;
        }

		if (rbot < rtop){
            for(i=rbot_pre; i < rbot; i++)
                ret += scnprintf(buf + ret, size - ret, "0");
            for(i=rbot ; i <= rtop; i++)
                ret += scnprintf(buf + ret, size - ret, "1");
            rbot_pre = rtop+1;
        }

		rbot = cur;
	}

    while(++rtop < nbits){
        ret += scnprintf(buf + ret, size - ret, "0");
    }
	return ret;
}


static int __init test_init(void)
{
    char buf[BMAP_SIZE] = {0,};
    char buf_temp[BMAP_SIZE+1] = {0,};
    unsigned long value=1;
    DECLARE_BITMAP(bmap_test2, BMAP_SIZE);

    bitmap_zero(bmap_test2, PBUTIL_BMAP_SIZE);
    bitmap_set(bmap_test2,8, 4);
    bitmap_set(bmap_test2,16, 8);
    bitmap_set(bmap_test2,504, 4);
    bitmap_scnprintf(bmap_test2,BMAP_SIZE,buf,BMAP_SIZE);
    trace_printk("%s \n",buf);   

    bitmap_print(bmap_test2,BMAP_SIZE,buf_temp,BMAP_SIZE+1);
    trace_printk("%s \n",buf_temp);   

    if(!kstrtoul(buf_temp,0,&value))
        trace_printk("%lx \n",value);   

	return 0;
}
#endif 

char * const pbstat_name[SON_PB_MAX] = {
	 "White",
	 "Blue",
	 "Green",
	 "Yellow",
	 "Red",
	 "Unmovable",
     "Isolated(migrate)",
     "Isolated(free)"
};

struct son_contig_page_info {
	unsigned long free_pages;
	unsigned long free_blocks_total;
	unsigned long free_blocks_suitable;
};


static void son_fill_contig_page_info(struct zone *zone,
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

/*
 * A fragmentation index only makes sense if an allocation of a requested
 * size would fail. If that is true, the fragmentation index indicates
 * whether external fragmentation or a lack of memory was the problem.
 * The value can be used to determine if page reclaim or compaction
 * should be used
 */
static int __son_fragmentation_index(unsigned int order, struct son_contig_page_info *info)
{
	unsigned long requested = 1UL << order;

	if (!info->free_blocks_total)
		return 0;

	/* Fragmentation index only makes sense when a request would fail */
	if (info->free_blocks_suitable)
		return -1000;

	/*
	 * Index is between 0 and 1 so return within 3 decimal places
	 *
	 * 0 => allocation would fail due to lack of memory
	 * 1 => allocation would fail due to fragmentation
	 */
	return 1000 - div_u64( (1000+(div_u64(info->free_pages * 1000ULL, requested))), info->free_blocks_total);
}

/* Same as __fragmentation index but allocs contig_page_info on stack */
int son_fragmentation_index(struct zone *zone, unsigned int order)
{
	struct son_contig_page_info info;

	son_fill_contig_page_info(zone, order, &info);
	return __son_fragmentation_index(order, &info);
}

static int __init test_init(void)
{
    pg_data_t *pgdat = NULL;
    struct zone *zone = NULL;
    int nid, zoneid;

    for_each_online_node(nid){
        pgdat = NODE_DATA(nid);
#if 0
        for (zoneid = 0; zoneid < MAX_NR_ZONES; zoneid++) {

            zone = &pgdat->node_zones[zoneid];
            if (!populated_zone(zone))
                continue;
            trace_printk("%10s : %lu ~ %lu \n",zone->name, zone->zone_start_pfn, zone_end_pfn(zone));
        }
#endif
        zone = &pgdat->node_zones[ZONE_NORMAL];
        trace_printk("%d \n", son_fragmentation_index(zone, pageblock_order));
#if 0 
        for (zoneid = 0; zoneid < MAX_NR_ZONES; zoneid++) {

            zone = &pgdat->node_zones[zoneid];
            if (!populated_zone(zone))
                continue;

            trace_printk("son_module, %10s : %d, %10s : %d, %10s : %d, %10s : %d \n",
                    pbstat_name[SON_PB_ISOMG],
                    zone->son_pbutil_list[SON_PB_ISOMG].cur_count,
                    pbstat_name[SON_PB_ISOFR],
                    zone->son_pbutil_list[SON_PB_ISOFR].cur_count,
                    pbstat_name[SON_PB_BLUE],
                    zone->son_pbutil_list[SON_PB_BLUE].cur_count,
                    pbstat_name[SON_PB_YELLOW],
                    zone->son_pbutil_list[SON_PB_YELLOW].cur_count);
        }
#endif
    }
       
	return 0;
}


static void __exit test_exit(void)
{

}

module_init(test_init)
module_exit(test_exit)
MODULE_LICENSE("GPL");
