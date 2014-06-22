#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/sched.h>

struct proc_dir_entry *my_dir;
struct proc_dir_entry *attr;

//read(fd, buf, len);
//kernle-->page[4K]
//read_att memcpy--->page
//copy_to_user(buf, page, len)

//cat /proc/mydir/time

int read_att(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int ret = 0;
	struct timespec spec1, spec2;
	
	ret += sprintf(page, "HelloWord %s\n", current->comm);		
	getnstimeofday(&spec1);
	//ssleep(5);
	msleep(50);
	//set_current_state
	//schdule_timeout
	getnstimeofday(&spec2);
	ret += sprintf(page + ret, "Current:%lus %luns\n", spec1.tv_sec, spec1.tv_nsec);
	ret += sprintf(page + ret, "Current:%lus %luns\n", spec2.tv_sec, spec2.tv_nsec);

	return ret;
}

static __init int time_test_init(void)
{
	int ret;
		
	//jiffies==-5*60*HZ

	printk("HZ = %d  jiffies = %ld\n", HZ, jiffies);
	printk("time = %ld\n", jiffies + 5 * 60 * HZ);

	my_dir = proc_mkdir("mydir", NULL);// /proc/mydir
	if(!my_dir)
		return -ENOMEM;
	attr = create_proc_read_entry("time", 0, my_dir, read_att, NULL);
	if(!attr){
		ret = -ENOMEM;
		goto create_att_error;
	}

	return 0;
create_att_error:
	remove_proc_entry("mydir", NULL);	
	return ret;
}

static __exit void time_test_exit(void)
{
	remove_proc_entry("time", my_dir);	
	remove_proc_entry("mydir", NULL);	
}

module_init(time_test_init);
module_exit(time_test_exit);

MODULE_LICENSE("GPL");
