#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>

struct tasklet_struct mytask;

void do_task(unsigned long data)
{
	printk("hello data = %lu\n", data);
}

irqreturn_t do_eint0(int no, void *data)
{
	printk("hello eint0 %d\n", no);

	tasklet_schedule(&mytask);

	return IRQ_HANDLED;//IRQ_NONE;	
}

static __init int irq_test_init(void)
{
	int ret;

	ret = request_irq(IRQ_EINT(0), do_eint0, IRQF_TRIGGER_FALLING, "eint0", NULL);
	if(ret < 0)
		return ret;

	tasklet_init(&mytask, do_task, 100);

	return 0;
}

static __exit void irq_test_exit(void)
{
	free_irq(IRQ_EINT(0), NULL);
}

module_init(irq_test_init);
module_exit(irq_test_exit);

MODULE_LICENSE("GPL");
