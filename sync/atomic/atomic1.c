#include <linux/init.h>
#include <linux/module.h>
#include <linux/preempt.h>
#include <linux/delay.h>
#include <asm/atomic.h>
#include <asm/bitops.h>

static __init int module_test_init(void)
{
	atomic_t a;
	unsigned long b = 0xffff0000;
	int c;

	printk("__LINUX_ARM_ARCH__ = %d\n", __LINUX_ARM_ARCH__);

	atomic_set(&a, 10);

	printk("a = %d\n", atomic_inc_return(&a));

	set_bit(3, &b);
	clear_bit(31, &b);

	printk("b = %lx\n", b);

	c = test_and_set_bit(0, &b);
	printk("b = %lx c = %d\n", b, c);

	//a = 11
	//b = 7fff0008
	//b = 7fff0009 c = 0

	return 0;
}

static __exit void module_test_exit(void)
{

}

module_init(module_test_init);
module_exit(module_test_exit);

MODULE_LICENSE("GPL");
