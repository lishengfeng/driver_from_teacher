#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include "s3c_led.h"

int led_match(struct device *dev, struct device_driver *drv)
{
	struct led_dev *ldev = container_of(dev, struct led_dev, dev);
	struct led_drv *ldrv = container_of(drv, struct led_drv, drv);

	return !strcmp(ldev->colour, ldrv->colour);
}

struct led_bus lbus = {
	.name = "ledbus",
	.bus = {
		.name = "ledbus",
		.match = led_match,
	}
};
EXPORT_SYMBOL(lbus);

static __init int bus_test_init(void)
{
	return bus_register(&lbus.bus);
}

static __exit void bus_test_exit(void)
{
	bus_unregister(&lbus.bus);
}

module_init(bus_test_init);
module_exit(bus_test_exit);

MODULE_LICENSE("GPL");
