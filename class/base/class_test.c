#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/err.h>

struct class *led_class;
struct device *led_class_dev;

static __init int dev_test_init(void)
{
	int ret;

	led_class = class_create(THIS_MODULE, "led_class");
	if(PTR_RET(led_class)){
		ret = PTR_RET(led_class);
		goto class_error;
	}

	led_class_dev = device_create(led_class, NULL, MKDEV(250, 10), NULL, "hello-led");	
	if(PTR_RET(led_class_dev)){
		ret = PTR_RET(led_class_dev);
		goto dev_error;
	}

	return 0;
dev_error:
	class_destroy(led_class);
class_error:
	return ret;
}

static __exit void dev_test_exit(void)
{
	device_destroy(led_class, MKDEV(250, 10));
	class_destroy(led_class);
}

module_init(dev_test_init);
module_exit(dev_test_exit);

MODULE_LICENSE("GPL");





