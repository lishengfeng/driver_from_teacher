#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>

struct work_struct work;
struct delayed_work dwork;

void do_work(struct work_struct *work)
{
	printk("hello %s\n", __FUNCTION__);
}

void do_dwork(struct work_struct *work)
{
	printk("hello %s\n", __FUNCTION__);
}

irqreturn_t do_eint0(int no, void *data)
{
	printk("hello eint0 %d\n", no);

	schedule_work(&work);
	schedule_delayed_work(&dwork, 4 * HZ);

	return IRQ_HANDLED;//IRQ_NONE;	
}

static __init int irq_test_init(void)
{
	int ret;

	ret = request_irq(IRQ_EINT(0), do_eint0, IRQF_TRIGGER_FALLING, "eint0", NULL);
	if(ret < 0)
		return ret;

	INIT_WORK(&work, do_work);
	INIT_DELAYED_WORK(&dwork, do_dwork);

	return 0;
}

static __exit void irq_test_exit(void)
{
	flush_scheduled_work();//睡眠等待被调度过的工作执行完
	free_irq(IRQ_EINT(0), NULL);
}

module_init(irq_test_init);
module_exit(irq_test_exit);

MODULE_LICENSE("GPL");
