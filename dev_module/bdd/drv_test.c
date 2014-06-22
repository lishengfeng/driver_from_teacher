#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

extern struct bus_type mybus;

//设备和驱动匹配成功调用，把和该驱动匹配成功的设备传进来
int my_probe(struct device *dev)
{
	printk("probe\n");

	return 0;
}

//设备或者驱动注销的时候调用
int my_remove(struct device *dev)
{
	printk("remove\n");

	return 0;
}

//关机的时候调用
void my_shutdown(struct device *dev)
{
}

//睡眠时候调用
int my_suspend(struct device *dev, pm_message_t state)
{
	return 0;
}

//从睡眠中回复的时候调
int my_resume(struct device *dev)
{
	return 0;
}

struct device_driver mydrv = {
	.name = "world",
	.bus = &mybus,
	.probe = my_probe,
	.remove = my_remove,
	.shutdown = my_shutdown,
	.suspend = my_suspend,
	.resume = my_resume,
};

static __init int drv_test_init(void)
{
	return driver_register(&mydrv);
}

static __exit void drv_test_exit(void)
{
	driver_unregister(&mydrv);
}

module_init(drv_test_init);
module_exit(drv_test_exit);

MODULE_LICENSE("GPL");





