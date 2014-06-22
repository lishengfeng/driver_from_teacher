#include <linux/init.h>
#include <linux/module.h>

extern void my_printk(int no);

static __init int module_test_init(void)
{
	my_printk(100);
	
	return 0;
}

static __exit void module_test_exit(void)
{

}

module_init(module_test_init);
module_exit(module_test_exit);

MODULE_LICENSE("GPL");
