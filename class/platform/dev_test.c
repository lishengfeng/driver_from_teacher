#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include "s3c_led.h"

struct led_colour lc = {"blue"};

void led_release(struct device *dev)
{

}

struct resource led_res[] = {
	[0] = {
		.start = S3C_PA_LED,
		.end = S3C_PA_LED + S3C_SZ_LED - 1,
		.name = "led_reg",
		.flags = IORESOURCE_MEM,
	},
};

struct platform_device dev = {
	.name = "s3c-led", 
	.id = -1,
	.num_resources = ARRAY_SIZE(led_res),
	.resource = led_res,
	.dev = {
		.release = led_release,
		.platform_data = &lc,
	}
};

static __init int dev_test_init(void)
{
	return platform_device_register(&dev);
}

static __exit void dev_test_exit(void)
{
	platform_device_unregister(&dev);
}

module_init(dev_test_init);
module_exit(dev_test_exit);

MODULE_LICENSE("GPL");





