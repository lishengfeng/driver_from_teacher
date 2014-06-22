#include <linux/init.h>
#include <linux/module.h>
#include <linux/preempt.h>
#include <linux/delay.h>

static __init int module_test_init(void)
{
	//while(1);
	//前提是内核支持内核强制
	preempt_disable();
	mdelay(3000);
	preempt_enable();

	return 0;
}

static __exit void module_test_exit(void)
{

}

module_init(module_test_init);
module_exit(module_test_exit);

MODULE_LICENSE("GPL");
