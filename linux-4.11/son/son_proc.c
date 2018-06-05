#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/tty.h>      
#include <linux/console.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/rmap.h>
#include <linux/kernel.h>
#include <asm/mman.h>
#include <linux/huge_mm.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>

#include <son/son.h>

#define SON_BUFLEN          64
#define SON_BUFLEN_LONG     SON_BUFLEN*2

/* proc/son directory  */
struct proc_dir_entry* son_parent_dir;

#if SON_PBSCAND_ENABLE
/* proc/son/scan_pbstate_enable  */
struct proc_dir_entry* son_scan_pbstat_enable_data; 
atomic_t son_scan_pbstate_enable;	

static ssize_t son_read_pbstat_enable(struct file *filep, 
        char __user *buf, size_t size, loff_t *ppos)
{
	char buf_read[SON_BUFLEN];
	ssize_t len;

	len = scnprintf(buf_read, SON_BUFLEN, "%d\n", atomic_read(&son_scan_pbstate_enable));

	return simple_read_from_buffer(buf, size, ppos, buf_read, len);
}

static ssize_t son_write_pbstat_enable(struct file *filep, 
		const char __user *buf, size_t size, loff_t *ppos)
{
    char buf_write[SON_BUFLEN];
    long temp_write;
    size_t len_written;

    if(size > SON_BUFLEN)
        size = SON_BUFLEN;

    memset(buf_write,0,SON_BUFLEN);
    len_written = simple_write_to_buffer(buf_write,SON_BUFLEN,ppos,buf,size);

    if(!kstrtol(buf_write,0,&temp_write)){ 
        if(temp_write > 0){
            if(!atomic_read(&son_scan_pbstate_enable)){
                /* if current scanning state is not enabled make it enabled*/
                atomic_set(&son_scan_pbstate_enable,SON_ENABLE);
#if SON_PBSCAND_ENABLE
                wake_up_interruptible(&son_scand_pbstate_wait);
#endif
            }
        }else if(temp_write == 0){
            if(atomic_read(&son_scan_pbstate_enable)){
                /* if current scanning state is enabled make it disabled */ 
                atomic_set(&son_scan_pbstate_enable,SON_DISABLE);

            }
        }else if(temp_write < 0){
            /* error stat : data from user is negative value */
            return -EINVAL;
        }
    }
    return len_written;
}


static struct file_operations son_scan_pbstat_enable_ops = {
	.read = son_read_pbstat_enable,
    .write = son_write_pbstat_enable,    
	.llseek = generic_file_llseek,
};


#endif

#if SON_DEBUG_ENABLE
/* proc/son/debug  */
struct proc_dir_entry* son_debug_data;
atomic_t son_debug_enable;	

static ssize_t son_write_debug_enable(struct file *filep, 
		const char __user *buf, size_t size, loff_t *ppos)
{
    char buf_write[SON_BUFLEN];
    long temp_write;
    size_t len_written;

    if(size > SON_BUFLEN)
        size = SON_BUFLEN;

    memset(buf_write,0,SON_BUFLEN);
    len_written = simple_write_to_buffer(buf_write,SON_BUFLEN,ppos,buf,size);

    if(!kstrtol(buf_write,0,&temp_write)){ 
        if(temp_write > 0){
            if(!atomic_read(&son_debug_enable)){
                atomic_set(&son_debug_enable,SON_ENABLE);
            }
        }else if(temp_write == 0){
            if(atomic_read(&son_debug_enable)){
                atomic_set(&son_debug_enable,SON_DISABLE);
            }
        }else if(temp_write < 0){
            return -EINVAL;
        }
    }
    return len_written;
}

static ssize_t son_read_debug_enable(struct file *filep, 
        char __user *buf, size_t size, loff_t *ppos)
{
	char buf_read[SON_BUFLEN];
	ssize_t len;

	len = scnprintf(buf_read, SON_BUFLEN, "%d\n", atomic_read(&son_debug_enable));

	return simple_read_from_buffer(buf, size, ppos, buf_read, len);
}

static struct file_operations son_debug_ops = {
	.read = son_read_debug_enable,
    .write = son_write_debug_enable,    
	.llseek = generic_file_llseek,
};


#endif

#if SON_REFSCAND_ENABLE
/* proc/son/scan_refcount_enable */
struct proc_dir_entry* son_scan_refcount_enable_data;
atomic_t son_scan_refcount_enable;

static ssize_t son_read_refcount_enable(struct file *filep, 
        char __user *buf, size_t size, loff_t *ppos)
{
	char buf_read[SON_BUFLEN];
	ssize_t len;

	len = scnprintf(buf_read, SON_BUFLEN, "%d\n", atomic_read(&son_scan_refcount_enable));

	return simple_read_from_buffer(buf, size, ppos, buf_read, len);

}

static ssize_t son_write_refcount_enable(struct file *filep, 
		const char __user *buf, size_t size, loff_t *ppos)
{
    char buf_write[SON_BUFLEN];
    long temp_write;
    size_t len_written;

    if(size > SON_BUFLEN)
        size = SON_BUFLEN;

    memset(buf_write,0,SON_BUFLEN);
    len_written = simple_write_to_buffer(buf_write,SON_BUFLEN,ppos,buf,size);

    if(!kstrtol(buf_write,0,&temp_write)){
        if(temp_write > 0){
            if(!atomic_read(&son_scan_refcount_enable)){
                /* if current scanning state is not enabled make it enabled*/
                atomic_set(&son_scan_refcount_enable,SON_ENABLE);
                wake_up_interruptible(&son_scand_refcount_wait);
            }
        }else if(temp_write == 0){
            if(atomic_read(&son_scan_refcount_enable)){
                /* if current scanning state is enabled make it disabled */ 
                atomic_set(&son_scan_refcount_enable,SON_DISABLE);
            }
        }else if(temp_write < 0){
            /* error stat : data from user is negative value */
            return -EINVAL;
        }
    }
    return len_written;
}

static struct file_operations son_scan_refcount_enable_ops = {
	.read = son_read_refcount_enable,
    .write = son_write_refcount_enable,    
	.llseek = generic_file_llseek,
};


#endif

#if SON_PBSTAT_ENABLE
/* proc/son/son_pbstat_show */
struct proc_dir_entry* son_pbstat_show_data;
struct proc_dir_entry* son_pbstat_show_raw_data;
struct proc_dir_entry* son_pbstat_show_debug_data;
struct proc_dir_entry* son_pbstat_comp_mig_level_data;
struct proc_dir_entry* son_pbstat_comp_free_level_data;
struct proc_dir_entry* son_pbstat_comp_mode_data;
struct proc_dir_entry* son_pbstat_mig_threshold_data;

atomic_t son_pbstat_comp_mig_level;
atomic_t son_pbstat_comp_free_level;
atomic_t son_pbstat_comp_mode;
atomic_t son_pbstat_mig_threshold;

char * const pbstat_names[SON_PB_MAX] = {
	 "White",
	 "Blue",
	 "Green",
	 "Yellow",
	 "Red",
	 "Unmovable",
     "Isolated(migrate)",
     "Isolated(free)"
};

char * const comp_mode_names[SON_COMPMODE_MAX] = {
	 "Default",
	 "PageBlockCompaction",
	 "Default_monitor",
};


static void *pbstat_start(struct seq_file *m, loff_t *pos)
{
	pg_data_t *pgdat;
	loff_t node = *pos;

	for (pgdat = first_online_pgdat();
	     pgdat && node;
	     pgdat = next_online_pgdat(pgdat))
		--node;

	return pgdat;
}

static void *pbstat_next(struct seq_file *m, void *arg, loff_t *pos)
{
	pg_data_t *pgdat = (pg_data_t *)arg;

	(*pos)++;
	return next_online_pgdat(pgdat);
}

static void pbstat_stop(struct seq_file *m, void *arg)
{
}

/* Walk all the zones in a node and print using a callback 
 * (same as walk_zones_in_node in mm/vmstat.c) */
static void son_walk_zones_in_node(struct seq_file *m, pg_data_t *pgdat,
		void (*print)(struct seq_file *m, pg_data_t *, struct zone *))
{
	struct zone *zone;
	struct zone *node_zones = pgdat->node_zones;
	unsigned long flags;

	for (zone = node_zones; zone - node_zones < MAX_NR_ZONES; ++zone) {
		if (!populated_zone(zone))
			continue;

		spin_lock_irqsave(&zone->lock, flags);
		print(m, pgdat, zone);
		spin_unlock_irqrestore(&zone->lock, flags);
	}
}

static void pbstat_show_print(struct seq_file *m, pg_data_t *pgdat, struct zone *zone)
{
	int index;
//    spin_lock(&zone->pbutil_list_lock);
	seq_printf(m, "Node %d(%d), zone %8s - ", pgdat->node_id, pgdat->son_pbutil_tree.node_count, zone->name);

	for (index = 0; index < SON_PB_MAX; ++index)
		seq_printf(m, "%10s(%5d) ", pbstat_names[index],zone->son_pbutil_list[index].cur_count);       
	seq_putc(m, '\n');
//    spin_unlock(&zone->pbutil_list_lock);

}

static int pbstat_show(struct seq_file *m, void *arg)
{
	pg_data_t *pgdat = (pg_data_t *)arg;
	son_walk_zones_in_node(m, pgdat, pbstat_show_print);
	return 0;
}

static struct seq_operations son_pbstat_show_ops = {
	.start	= pbstat_start, 
    /* iterate over all zones. (same as frag_start in mm/vmstat.c) */
	.next = pbstat_next,
    /* select next online node (same as frag_next in mm/vmstat.c) */
	.stop = pbstat_stop,
    /* dummy...  */    
	.show = pbstat_show
    /* show page block utilization info (similar to frag_next in mm/vmstat.c) */
};

static int son_pbstat_show_open(struct inode *inode, struct file *file) 
{
    return seq_open(file, &son_pbstat_show_ops);
}

static struct file_operations son_pbstat_show_enable_ops = {
	.open = son_pbstat_show_open,
	.read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};

static void pbstat_show_raw_print(struct seq_file *m, pg_data_t *pgdat, struct zone *zone)
{
	int index;

	seq_printf(m, "%d,%8s", pgdat->node_id, zone->name);
    
	for (index = 0; index < SON_PB_MAX; ++index)
		seq_printf(m, ",%5d",zone->son_pbutil_list[index].cur_count);
        
	seq_putc(m, '\n');
}

static int pbstat_show_raw(struct seq_file *m, void *arg)
{
	pg_data_t *pgdat = (pg_data_t *)arg;
	son_walk_zones_in_node(m, pgdat, pbstat_show_raw_print);
	return 0;
}

static struct seq_operations son_pbstat_show_raw_ops = {
	.start	= pbstat_start, 
    /* iterate over all zones. (same as frag_start in mm/vmstat.c) */
	.next = pbstat_next,
    /* select next online node (same as frag_next in mm/vmstat.c) */
	.stop = pbstat_stop,
    /* dummy...  */    
	.show = pbstat_show_raw
    /* show page block utilization info (similar to frag_next in mm/vmstat.c) */
};

static int son_pbstat_show_raw_open(struct inode *inode, struct file *file) 
{
    return seq_open(file, &son_pbstat_show_raw_ops);
}

static struct file_operations son_pbstat_show_raw_enable_ops = {
	.open = son_pbstat_show_raw_open,
	.read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};

/* same function in /tools/lib/bitmap.c  */
size_t son_bitmap_scnprintf(unsigned long *bitmap, int nbits,
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

static void pbstat_show_debug_print(struct seq_file *m, pg_data_t *pgdat, struct zone *zone)
{
    pbutil_tree_t *pbutil_tree = NULL;
    pbutil_list_t *pbutil_list = NULL;
    pbutil_node_t *pbutil_node = NULL;
	int level;

    pbutil_tree = &pgdat->son_pbutil_tree;
	seq_printf(m, "Node %d, zone %8s - total count : %d \n", 
            pgdat->node_id, zone->name, pbutil_tree->node_count);

//    spin_lock(&zone->pbutil_list_lock);    
    rcu_read_lock();
	for (level = 0; level < SON_PB_MAX; ++level){
        pbutil_list = &zone->son_pbutil_list[level];
        seq_printf(m, "%10s - count : %10d \n", 
                pbstat_names[level],pbutil_list->cur_count);

        list_for_each_entry_rcu(pbutil_node, 
                &pbutil_list->pbutil_list_head,pbutil_level){

            char buf_temp[PBUTIL_BMAP_SIZE+1] = {0,};
            bitmap_print(pbutil_node->pbutil_movable_bitmap,
                    PBUTIL_BMAP_SIZE,buf_temp,PBUTIL_BMAP_SIZE+1);

            seq_printf(m, "%10s - %10lu, %5lu, %5lu, %s \n", 
                    pbstat_names[level], pbutil_node->pb_head_pfn,
                    pbutil_node->used_movable_page, 
                    pbutil_node->used_unmovable_page, buf_temp);
        }
    }
    rcu_read_unlock();
//    spin_unlock(&zone->pbutil_list_lock);       
}

static int pbstat_show_debug(struct seq_file *m, void *arg)
{
	pg_data_t *pgdat = (pg_data_t *)arg;
    son_walk_zones_in_node(m, pgdat, pbstat_show_debug_print);
	return 0;
}

static struct seq_operations son_pbstat_show_debug_ops = {
	.start	= pbstat_start, 
    /* iterate over all zones. (same as frag_start in mm/vmstat.c) */
	.next = pbstat_next,
    /* select next online node (same as frag_next in mm/vmstat.c) */
	.stop = pbstat_stop,
    /* dummy...  */    
	.show = pbstat_show_debug
    /* show page block utilization info (similar to frag_next in mm/vmstat.c) */
};

static int son_pbstat_show_debug_open(struct inode *inode, struct file *file) 
{
    return seq_open(file, &son_pbstat_show_debug_ops);
}

static struct file_operations son_pbstat_show_debug_enable_ops = {
	.open = son_pbstat_show_debug_open,
	.read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};

static ssize_t son_read_pbstat_comp_mig_level(struct file *filep, 
        char __user *buf, size_t size, loff_t *ppos)
{
	char buf_read[SON_BUFLEN_LONG];
	ssize_t len;
    int cur_level = atomic_read(&son_pbstat_comp_mig_level);
	len = scnprintf(buf_read, SON_BUFLEN_LONG, "current level : %10s (1:Blue/2:Green/3:Yellow, must be smaller than pbstat_compact_free_level) \n", pbstat_names[cur_level]);

	return simple_read_from_buffer(buf, size, ppos, buf_read, len);

}

static ssize_t son_write_pbstat_comp_mig_level(struct file *filep, 
		const char __user *buf, size_t size, loff_t *ppos)
{
    char buf_write[SON_BUFLEN];
    long temp_write, cur_free_level, cur_mig_level;
    size_t len_written;

    if(size > SON_BUFLEN)
        size = SON_BUFLEN;

    memset(buf_write,0,SON_BUFLEN);
    cur_free_level = atomic_read(&son_pbstat_comp_free_level);
    cur_mig_level = atomic_read(&son_pbstat_comp_mig_level);

    len_written = simple_write_to_buffer(buf_write,SON_BUFLEN,ppos,buf,size);

    if(!kstrtol(buf_write,0,&temp_write)){
        if(temp_write >= SON_PB_SYSFS_MIN && 
                temp_write <= SON_PB_SYSFS_MAX && 
                temp_write < cur_free_level){
            if(temp_write != cur_mig_level){
                /* if current scanning state is not enabled make it enabled*/
                atomic_set(&son_pbstat_comp_mig_level,temp_write);
            }
        }else{
            /* error stat : data from user is negative value */
            return -EINVAL;
        }
    }
    return len_written;
}

static struct file_operations son_pbstat_comp_mig_level_ops = {
	.read = son_read_pbstat_comp_mig_level,
    .write = son_write_pbstat_comp_mig_level,    
	.llseek = generic_file_llseek,
};

static ssize_t son_read_pbstat_comp_free_level(struct file *filep, 
        char __user *buf, size_t size, loff_t *ppos)
{
	char buf_read[SON_BUFLEN_LONG];
	ssize_t len;
    int cur_level = atomic_read(&son_pbstat_comp_free_level);
	len = scnprintf(buf_read, SON_BUFLEN_LONG, "current level : %10s (1:Blue/2:Green/3:Yellow, must be bigger than pbstat_compact_mig_level) \n", pbstat_names[cur_level]);

	return simple_read_from_buffer(buf, size, ppos, buf_read, len);

}

static ssize_t son_write_pbstat_comp_free_level(struct file *filep, 
		const char __user *buf, size_t size, loff_t *ppos)
{
    char buf_write[SON_BUFLEN];
    long temp_write, cur_free_level, cur_mig_level;
    size_t len_written;

    if(size > SON_BUFLEN)
        size = SON_BUFLEN;

    memset(buf_write,0,SON_BUFLEN);
    cur_free_level = atomic_read(&son_pbstat_comp_free_level);
    cur_mig_level = atomic_read(&son_pbstat_comp_mig_level);

    len_written = simple_write_to_buffer(buf_write,SON_BUFLEN,ppos,buf,size);

    if(!kstrtol(buf_write,0,&temp_write)){
        if(temp_write >= SON_PB_SYSFS_MIN && 
                temp_write <= SON_PB_SYSFS_MAX && 
                temp_write > cur_mig_level){
            if(temp_write != cur_free_level){
                /* if current scanning state is not enabled make it enabled*/
                atomic_set(&son_pbstat_comp_free_level,temp_write);
            }
        }else{
            /* error stat : data from user is negative value */
            return -EINVAL;
        }
    }
    return len_written;
}

static struct file_operations son_pbstat_comp_free_level_ops = {
	.read = son_read_pbstat_comp_free_level,
    .write = son_write_pbstat_comp_free_level,    
	.llseek = generic_file_llseek,
};


static ssize_t son_read_pbstat_comp_mode(struct file *filep, 
        char __user *buf, size_t size, loff_t *ppos)
{
	char buf_read[SON_BUFLEN];
	ssize_t len;
    int cur_mode = atomic_read(&son_pbstat_comp_mode);
    len = scnprintf(buf_read, SON_BUFLEN, "%10s (0:Default/1:PBC/2:Default_monitor) \n", comp_mode_names[cur_mode]);

	return simple_read_from_buffer(buf, size, ppos, buf_read, len);

}

static ssize_t son_write_pbstat_comp_mode(struct file *filep, 
		const char __user *buf, size_t size, loff_t *ppos)
{
    char buf_write[SON_BUFLEN];
    long temp_write;
    size_t len_written;

    if(size > SON_BUFLEN)
        size = SON_BUFLEN;

    memset(buf_write,0,SON_BUFLEN);
    len_written = simple_write_to_buffer(buf_write,SON_BUFLEN,ppos,buf,size);

    if(!kstrtol(buf_write,0,&temp_write)){
        if(temp_write >= SON_COMPMODE_ORIGIN && 
                temp_write <= SON_COMPMODE_ORIGIN_MONITOR){
            if(atomic_read(&son_pbstat_comp_mode) != temp_write){
                atomic_set(&son_pbstat_comp_mode,temp_write);
            }
        }else{
            /* error stat : data from user is negative value */
            return -EINVAL;
        }
    }
    return len_written;
}

static struct file_operations son_pbstat_comp_mode_ops = {
	.read = son_read_pbstat_comp_mode,
    .write = son_write_pbstat_comp_mode,
	.llseek = generic_file_llseek,
};

static ssize_t son_read_mig_threshold(struct file *filep, 
        char __user *buf, size_t size, loff_t *ppos)
{
	char buf_read[SON_BUFLEN];
	ssize_t len;
    long cur_value = atomic_read(&son_pbstat_mig_threshold);
    len = scnprintf(buf_read, SON_BUFLEN, "%ld \n", cur_value);

	return simple_read_from_buffer(buf, size, ppos, buf_read, len);

}

static ssize_t son_write_mig_threshold(struct file *filep, 
		const char __user *buf, size_t size, loff_t *ppos)
{
    char buf_write[SON_BUFLEN];
    long temp_write;
    size_t len_written;

    if(size > SON_BUFLEN)
        size = SON_BUFLEN;

    memset(buf_write,0,SON_BUFLEN);
    len_written = simple_write_to_buffer(buf_write,SON_BUFLEN,ppos,buf,size);

    if(!kstrtol(buf_write,0,&temp_write)){
        if(temp_write > 0){
            if(atomic_read(&son_pbstat_mig_threshold) != temp_write){
                atomic_set(&son_pbstat_mig_threshold,temp_write);
            }
        }else{
            /* error stat : data from user is negative value */
            return -EINVAL;
        }
    }
    return len_written;
}

static struct file_operations son_pbstat_mig_threshold_ops = {
	.read = son_read_mig_threshold,
    .write = son_write_mig_threshold,
	.llseek = generic_file_llseek,
};

#endif

static int __init son_proc_init(void)
{

	/* init procfs */ 
    son_parent_dir = proc_mkdir("son",NULL);
    if(!son_parent_dir)
        goto out;

#if SON_PBSCAND_ENABLE
    atomic_set(&son_scan_pbstate_enable, SON_DISABLE);

    /*  page usage kernel thread related entry  */
	son_scan_pbstat_enable_data = proc_create("scan_pbstat_enable", 0, son_parent_dir, &son_scan_pbstat_enable_ops);
    if(!son_scan_pbstat_enable_data)
        goto out;
#endif 

#if SON_DEBUG_ENABLE
    atomic_set(&son_debug_enable,SON_DISABLE);

	son_debug_data = proc_create("debug", 0, son_parent_dir, &son_debug_ops);
    if(!son_debug_data){

        goto out;
    }

#endif

#if SON_REFSCAND_ENABLE
    atomic_set(&son_scan_refcount_enable, SON_DISABLE);

    /*  page frequency kernel thread related entry  */
	son_scan_refcount_enable_data = proc_create("scan_refcount_enable", 0, son_parent_dir, &son_scan_refcount_enable_ops);
    if(!son_scan_refcount_enable_data)
        goto out;
#endif

#if SON_PBSTAT_ENABLE
    /*  page block utilization info related entry  */
	son_pbstat_show_data = proc_create("pbstat_info_show", 0, son_parent_dir, &son_pbstat_show_enable_ops);
    if(!son_pbstat_show_data)
        goto out;

    /*  page block utilization info related entry (for plotting graph) */
	son_pbstat_show_raw_data = proc_create("pbstat_info_show_raw", 0, son_parent_dir, &son_pbstat_show_raw_enable_ops);
    if(!son_pbstat_show_raw_data)
        goto out;

    /*  page block utilization info related entry (for debugging) */
	son_pbstat_show_debug_data = proc_create("pbstat_info_show_debug", 0, son_parent_dir, &son_pbstat_show_debug_enable_ops);
    if(!son_pbstat_show_debug_data)
        goto out;

    /*  page block utilization based sysfs compaction threshold level.  */
    atomic_set(&son_pbstat_comp_mig_level, SON_PB_SYSFS_MIN);
	son_pbstat_comp_mig_level_data = proc_create("pbstat_compact_mig_level", 0, son_parent_dir, &son_pbstat_comp_mig_level_ops);
    if(!son_pbstat_comp_mig_level_data)
        goto out;

    /*  page block utilization based sysfs compaction threshold level.  */
    atomic_set(&son_pbstat_comp_free_level, SON_PB_SYSFS_MAX);
	son_pbstat_comp_free_level_data = proc_create("pbstat_compact_free_level", 0, son_parent_dir, &son_pbstat_comp_free_level_ops);
    if(!son_pbstat_comp_free_level_data)
        goto out;

    /*  page block utilization based sysfs compaction threshold level.  */
    atomic_set(&son_pbstat_comp_mode, SON_COMPMODE_ORIGIN);
	son_pbstat_comp_mode_data = proc_create("pbstat_compact_mode", 0, son_parent_dir, &son_pbstat_comp_mode_ops);
    if(!son_pbstat_comp_mode_data)
        goto out;

    /*  page block utilization based sysfs compaction threshold level.  */
    atomic_set(&son_pbstat_mig_threshold, 0);
	son_pbstat_mig_threshold_data = proc_create("pbstat_compact_mig_threshold", 0, son_parent_dir, &son_pbstat_mig_threshold_ops);
    if(!son_pbstat_mig_threshold_data)
        goto out;
#endif
    return 0;

out: 

#if SON_PBSTAT_ENABLE 
    if(son_pbstat_comp_mode_data)
        proc_remove(son_pbstat_comp_mode_data); 
    if(son_pbstat_comp_free_level_data)
        proc_remove(son_pbstat_comp_free_level_data);
    if(son_pbstat_comp_mig_level_data)
        proc_remove(son_pbstat_comp_mig_level_data);
    if(son_pbstat_show_debug_data)
        proc_remove(son_pbstat_show_debug_data);
    if(son_pbstat_show_raw_data)
        proc_remove(son_pbstat_show_raw_data);
    if(son_pbstat_show_data)
        proc_remove(son_pbstat_show_data);
#endif

#if SON_REFSCAND_ENABLE 
    if(son_scan_refcount_enable_data)
        proc_remove(son_scan_refcount_enable_data);
#endif

#if SON_DEBUG_ENABLE 
    if(son_debug_data)
        proc_remove(son_debug_data);
#endif

#if SON_PBSCAND_ENABLE
    if(son_scan_pbstat_enable_data)
        proc_remove(son_scan_pbstat_enable_data);
#endif
    if(son_parent_dir)
        proc_remove(son_parent_dir);

	return -ENOMEM;
}

subsys_initcall(son_proc_init);

