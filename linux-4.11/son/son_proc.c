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

#define SON_BUFLEN      12 

/* proc/son directory  */
struct proc_dir_entry* son_parent_dir;

#if SON_PBSCAND_ENABLE
/* proc/son/scan_pbstate_enable  */
struct proc_dir_entry* son_scan_pbstate_enable_data; 
atomic_t son_scan_pbstate_enable;	

static ssize_t son_read_pbstate_enable(struct file *filep, 
        char __user *buf, size_t size, loff_t *ppos)
{
	char buf_read[SON_BUFLEN];
	ssize_t len;

	len = scnprintf(buf_read, SON_BUFLEN, "%d\n", atomic_read(&son_scan_pbstate_enable));

	return simple_read_from_buffer(buf, size, ppos, buf_read, len);
}

static ssize_t son_write_pbstate_enable(struct file *filep, 
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
//                if(atomic_read(&son_debug_enable)){
//                    trace_printk("son - page reference counting is enabled \n"); 
//                    trace_printk("son - wakeup son_scand \n"); 
//                }
            }
        }else if(temp_write == 0){
            if(atomic_read(&son_scan_pbstate_enable)){
                /* if current scanning state is enabled make it disabled */ 
                atomic_set(&son_scan_pbstate_enable,SON_DISABLE);

//                if(atomic_read(&son_debug_enable)){
//                    trace_printk("son - page reference counting is disabled \n");
//                    trace_printk("son - sleep son_scand \n"); 
//                }
            }
        }else if(temp_write < 0){
            /* error stat : data from user is negative value */
//            if(atomic_read(&son_debug_enable)){
//                trace_printk("son - (err)page reference counting invalid write \n"); 
//            }
            return -EINVAL;
        }
    }
    return len_written;
}


static struct file_operations son_scan_pbstate_enable_ops = {
	.read = son_read_pbstate_enable,
    .write = son_write_pbstate_enable,    
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
struct proc_dir_entry* son_pbstat_show_data_raw;

char * const pbstat_names[SON_PB_MAX] = {
	 "White",
	 "Blue",
	 "Green",
	 "Yellow",
	 "Red",
	 "Unmovable",
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

	seq_printf(m, "Node %d, zone %8s - ", pgdat->node_id, zone->name);
    
	for (index = 0; index < SON_PB_MAX; ++index)
		seq_printf(m, "%10s(%5d) ", pbstat_names[index],zone->son_pbutil_list[index].cur_count);
        
	seq_putc(m, '\n');
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
	son_scan_pbstate_enable_data = proc_create("scan_pbstate_enable", 0, son_parent_dir, &son_scan_pbstate_enable_ops);
    if(!son_scan_pbstate_enable_data)
        goto out;
#endif 

#if SON_DEBUG_ENABLE
    atomic_set(&son_debug_enable,SON_DISABLE);

	son_debug_data = proc_create("debug", 0, son_parent_dir, &son_debug_ops);
    if(!son_debug_data)
        goto out;

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
	son_pbstat_show_data = proc_create("scan_pbstat_show", 0, son_parent_dir, &son_pbstat_show_enable_ops);
    if(!son_pbstat_show_data)
        goto out;

    /*  page block utilization info related entry  */
	son_pbstat_show_data_raw = proc_create("scan_pbstat_show_raw", 0, son_parent_dir, &son_pbstat_show_raw_enable_ops);
    if(!son_pbstat_show_data_raw)
        goto out;

#endif
    printk(KERN_DEBUG "proc interface for son is initialized");
    return 0;

out: 
#if SON_REFSCAND_ENABLE
    proc_remove(son_scan_refcount_enable_data);
#endif
#if SON_DEBUG_ENABLE
    proc_remove(son_debug_data);
#endif
#if SON_PBSCAND_ENABLE
    proc_remove(son_scan_pbstate_enable_data);
#endif
    proc_remove(son_parent_dir);
	return -ENOMEM;
}
subsys_initcall(son_proc_init);

