#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include "s3c_led.h"

extern struct led_bus lbus;

void led_release(struct device *dev)
{

}

struct led_dev ldev = {
	.colour = "red",
	.dev = {
		.init_name = "red",
		.bus = &lbus.bus,
		.release = led_release,
	}
};

static __init int dev_test_init(void)
{
	return device_register(&ldev.dev);
}

static __exit void dev_test_exit(void)
{
	device_unregister(&ldev.dev);
}

module_init(dev_test_init);
module_exit(dev_test_exit);

MODULE_LICENSE("GPL");





