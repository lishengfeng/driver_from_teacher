#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>

#include "s3c_ts.h"

//platform_device_unregister(&dev);
void ts_release(struct device *dev)
{

}

struct resource ts_res[] = {
	[0] = {
		.start = S3C_PA_TS,
		.end = S3C_PA_TS + S3C_SZ_TS - 1,
		.name = "s3c-ts-reg",
		.flags = IORESOURCE_MEM,
	},	
	[1] = {
		.start = IRQ_TC,
		.end = IRQ_ADC,
		.name = "s3c-ts-irq",
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device dev = {
	.id = -1,
	.name = "s3c-ts",
	.num_resources = ARRAY_SIZE(ts_res),
	.resource = ts_res,
	.dev = {
		.release = ts_release,
	}
};

static __init int ts_test_init(void)
{
	return platform_device_register(&dev);
}

static __exit void ts_test_exit(void)
{
	platform_device_unregister(&dev);
}

module_init(ts_test_init);
module_exit(ts_test_exit);

MODULE_LICENSE("GPL");
