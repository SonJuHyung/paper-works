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

/* proc/son/scan_pbstate_enable  */
struct proc_dir_entry* son_scan_pbstate_enable_data; 
/* proc/son/debug  */
struct proc_dir_entry* son_debug_data;
/* proc/son/scan_refcount_enable */
struct proc_dir_entry* son_scan_refcount_enable_data;

atomic_t son_scan_pbstate_enable;	
atomic_t son_debug_enable;	
atomic_t son_scan_refcount_enable;


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


struct file_operations son_scan_pbstate_enable_ops = {
	.read = son_read_pbstate_enable,
    .write = son_write_pbstate_enable,    
	.llseek = generic_file_llseek,
};

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
#if SON_REFSCAND_ENABLE
                wake_up_interruptible(&son_scand_refcount_wait);
#endif
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

struct file_operations son_scan_refcount_enable_ops = {
	.read = son_read_refcount_enable,
    .write = son_write_refcount_enable,    
	.llseek = generic_file_llseek,
};

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

struct file_operations son_debug_ops = {
	.read = son_read_debug_enable,
    .write = son_write_debug_enable,    
	.llseek = generic_file_llseek,
};

static int __init son_proc_init(void)
{
    atomic_set(&son_scan_pbstate_enable, SON_DISABLE);
    atomic_set(&son_debug_enable,SON_DISABLE);
    atomic_set(&son_scan_refcount_enable, SON_DISABLE);

	/* init procfs */ 
    son_parent_dir = proc_mkdir("son",NULL);
    if(!son_parent_dir)
        goto out;

    /*  page usage kernel thread related entry  */
	son_scan_pbstate_enable_data = proc_create("scan_pbstate_enable", 0, son_parent_dir, &son_scan_pbstate_enable_ops);
    if(!son_scan_pbstate_enable_data)
        goto out;

	son_debug_data = proc_create("debug", 0, son_parent_dir, &son_debug_ops);
    if(!son_debug_data)
        goto out;

    /*  page frequency kernel thread related entry  */
	son_scan_refcount_enable_data = proc_create("scan_refcount_enable", 0, son_parent_dir, &son_scan_refcount_enable_ops);
    if(!son_scan_refcount_enable_data)
        goto out;


    printk(KERN_DEBUG "proc interface for son is initialized");
    return 0;

out: 
    proc_remove(son_debug_data);
    proc_remove(son_scan_pbstate_enable_data);
    proc_remove(son_parent_dir);
	return -ENOMEM;
}
subsys_initcall(son_proc_init);

