#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

extern struct bus_type mybus;

void my_release(struct device *dev)
{

}

struct device mydev = {
	.init_name = "hello",
	.bus = &mybus,
	.release = my_release,
};

static __init int dev_test_init(void)
{
	return device_register(&mydev);
}

static __exit void dev_test_exit(void)
{
	device_unregister(&mydev);
}

module_init(dev_test_init);
module_exit(dev_test_exit);

MODULE_LICENSE("GPL");





