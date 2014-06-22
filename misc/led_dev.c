#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include "s3c_led.h"

struct led_platform_data led_data = {"Led Test For Class"};

void led_release(struct device *dev)
{

}

struct resource led_res[] = {
	[0] = {
		.start = S3C_PA_LED,
		.end = S3C_PA_LED + S3C_SZ_LED - 1,
		.name = "led_res",
		.flags = IORESOURCE_MEM,
	},
};

struct platform_device dev = {
	.name = "myled",
	.id = -1,
	.num_resources = ARRAY_SIZE(led_res),
	.resource = led_res,
	.dev = {
		.release = led_release,
		.platform_data = &led_data,
	}
};

static int __init led_test_init(void)
{
	return platform_device_register(&dev);
}

static void __exit led_test_exit(void)
{
	platform_device_unregister(&dev);
}

module_init(led_test_init);
module_exit(led_test_exit);

MODULE_LICENSE("GPL");

