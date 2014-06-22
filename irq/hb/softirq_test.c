#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>

//允许被其他函数打断
//但是软中断处理函数仍然处于中断上下文，不能睡眠
void my_soft(struct softirq_action *h)
{
	printk("-----%s-----\n", __FUNCTION__);
}

irqreturn_t do_eint0(int no, void *data)
{
	printk("hello eint0 %d\n", no);

	raise_softirq(MY_SOFTIRQ);

	return IRQ_HANDLED;//IRQ_NONE;	
}

static __init int irq_test_init(void)
{
	int ret;

	ret = request_irq(IRQ_EINT(0), do_eint0, IRQF_TRIGGER_FALLING, "eint0", NULL);
	if(ret < 0)
		return ret;

	open_softirq(MY_SOFTIRQ, my_soft);

	return 0;
}

static __exit void irq_test_exit(void)
{
	free_irq(IRQ_EINT(0), NULL);
}

module_init(irq_test_init);
module_exit(irq_test_exit);

MODULE_LICENSE("GPL");
