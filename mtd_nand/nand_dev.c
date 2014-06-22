#include <linux/init.h>
#include <linux/module.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/mtd/nand.h>
#include "s3c_nand.h"

struct mtd_partition nand_partitions[] = {
	[0] = {
		.name = "Boot",
		.size = SZ_1M,
		.offset = 0,
	},
	[1] = {
		.name = "Kernel",
		.size = SZ_4M,
		.offset = MTDPART_OFS_APPEND,
	},
	[2] = {
		.name = "RootFs",
		.size = SZ_128M,
		.offset = MTDPART_OFS_APPEND,
	},
	[3] = {
		.name = "Data",
		.size = MTDPART_SIZ_FULL,
		.offset = MTDPART_OFS_APPEND,
	},
};

struct nand_part_info nand_data = {
	.partition = nand_partitions,
	.num_partitions = ARRAY_SIZE(nand_partitions),
	.num_chips = 1,
};

struct resource nand_res[] = {
	[0] = {
		.start = S3C_PA_NAND, 
        .end = S3C_PA_NAND + S3C_SZ_NAND - 1, 
        .flags = IORESOURCE_MEM,
	},	
};

void nrelease(struct device *dev)
{

}

struct platform_device dev = {
	.name = "s3c-nand-flash",
	.id = -1,
	.resource = nand_res,
	.num_resources = ARRAY_SIZE(nand_res),
	.dev = {
		.release = nrelease,
		.platform_data = &nand_data,
	},
};

static __init int nand_test_init(void)
{
	return platform_device_register(&dev);
}

static __exit void nand_test_exit(void)
{
	platform_device_unregister(&dev);
}

module_init(nand_test_init);
module_exit(nand_test_exit);

MODULE_LICENSE("GPL");
