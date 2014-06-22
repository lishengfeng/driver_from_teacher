#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>

struct timer_list mytimer;

void do_timer(unsigned long data)
{
	printk("timer timer timer\n");

//	mytimer.expires = jiffies + data;
//	add_timer(&mytimer);
	mod_timer(&mytimer, jiffies + data);
}

static __init int time_test_init(void)
{
	//init_timer(&mytimer);
	//mytimer.expires = jiffies + 3 * HZ;
	//mytimer.function = do_timer;
	//mytimer.data = 100;
	//add_timer(&mytimer);	

	//setup_timer(&mytimer, do_timer, 100);
	//mytimer.expires = jiffies + 3 * HZ;
	//add_timer(&mytimer);

	setup_timer(&mytimer, do_timer, 100);
	mod_timer(&mytimer, jiffies + 300);

	return 0;
}

static __exit void time_test_exit(void)
{
	del_timer(&mytimer);
}

module_init(time_test_init);
module_exit(time_test_exit);

MODULE_LICENSE("GPL");
