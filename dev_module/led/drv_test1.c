#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include "s3c_led.h"

extern struct led_bus lbus;

//设备和驱动匹配成功调用，把和该驱动匹配成功的设备传进来
int led_probe(struct device *dev)
{
	struct led_dev *ldev = container_of(dev, struct led_dev, dev);

	printk("Device %s probe\n", ldev->colour);

	return 0;
}

//设备或者驱动注销的时候调用
int led_remove(struct device *dev)
{
	printk("remove\n");

	return 0;
}

//关机的时候调用
void led_shutdown(struct device *dev)
{
}

//睡眠时候调用
int led_suspend(struct device *dev, pm_message_t state)
{
	return 0;
}

//从睡眠中回复的时候调
int led_resume(struct device *dev)
{
	return 0;
}

struct led_drv ldrv = {
	.colour = "green",
	.drv = {
		.name = "green",
		.bus = &lbus.bus,
		.probe = led_probe,
		.remove = led_remove,
		.shutdown = led_shutdown,
		.suspend = led_suspend,
		.resume = led_resume,
	}
};

static __init int drv_test_init(void)
{
	return driver_register(&ldrv.drv);
}

static __exit void drv_test_exit(void)
{
	driver_unregister(&ldrv.drv);
}

module_init(drv_test_init);
module_exit(drv_test_exit);

MODULE_LICENSE("GPL");





