#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

int my_match(struct device *dev, struct device_driver *drv)
{
	return 1;
}

struct bus_type mybus = {
	.name = "mybus",
	.match = my_match,
};
EXPORT_SYMBOL(mybus);

static __init int bus_test_init(void)
{
	return bus_register(&mybus);
}

static __exit void bus_test_exit(void)
{
	bus_unregister(&mybus);
}

module_init(bus_test_init);
module_exit(bus_test_exit);

MODULE_LICENSE("GPL");
