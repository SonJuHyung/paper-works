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

static void __exit test_exit(void)
{

}

module_init(test_init)
module_exit(test_exit)
MODULE_LICENSE("GPL");
