#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>

#include "dm9000.h"

struct dm9000_platform_data dm9000_data = {
	.name = "My Name Is Dm9000",
	.eth_addr = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66},
};

//platform_device_unregister(&dev);
void dm9000_release(struct device *dev)
{

}

struct resource dm9000_res[] = {
	[0] = {
		.start = S3C_PA_BW,
		.end = S3C_PA_BW + S3C_SZ_BW - 1,
		.name = "sromc",
		.flags = IORESOURCE_MEM,
	},	
	[1] = {
		.start = S3C_PA_IO,
		.end = S3C_PA_IO + S3C_SZ_DM9000 - 1,
		.name = "dm9000",
		.flags = IORESOURCE_MEM,
	},
	[2] = {
		.start = IRQ_EINT(7),
		.end = IRQ_EINT(7),
		.name = "dm9000_irq",
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device dev = {
	.id = -1,
	.name = "s3c-dm9000",
	.num_resources = ARRAY_SIZE(dm9000_res),
	.resource = dm9000_res,
	.dev = {
		.release = dm9000_release,
		.platform_data = &dm9000_data,
	}
};

static __init int dm9000_test_init(void)
{
	return platform_device_register(&dev);
}

static __exit void dm9000_test_exit(void)
{
	platform_device_unregister(&dev);
}

module_init(dm9000_test_init);
module_exit(dm9000_test_exit);

MODULE_LICENSE("GPL");
