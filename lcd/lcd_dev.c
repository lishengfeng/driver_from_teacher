#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include "s3c_fb.h"

static void lcd_release(struct device *dev)
{

}

struct resource fb_res[] = {
	//mifpcon	
	[0] = {
		.start = S3C_PA_FB_MIF, 
		.end = S3C_PA_FB_MIF + S3C_SZ_FB_MIF - 1,
		.name = "mifpcon",
		.flags = IORESOURCE_MEM,
	},
	//gpio spcon
	[1] = {
		.start = S3C_PA_FB_GPIO, 
		.end = S3C_PA_FB_GPIO + S3C_SZ_FB_GPIO - 1,
		.name = "gpio",
		.flags = IORESOURCE_MEM,
	},
	//lcd
	[2] = {
		.start = S3C_PA_FB, 
		.end = S3C_PA_FB + S3C_SZ_FB - 1,
		.name = "lcd",
		.flags = IORESOURCE_MEM,
	},
};

struct platform_device dev = {
	.id = -1,
	.name = "my-s3c-fb",
	.num_resources = ARRAY_SIZE(fb_res),
	.resource = fb_res,
	.dev = {
		.release = lcd_release,
	}
};

static __init int fb_test_init(void)
{
	return platform_device_register(&dev);	
}

static __exit void fb_test_exit(void)
{
	platform_device_unregister(&dev);
}

module_init(fb_test_init);
module_exit(fb_test_exit);

MODULE_LICENSE("GPL");
