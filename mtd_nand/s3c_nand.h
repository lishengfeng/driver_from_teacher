#ifndef __S3C_NAND_H
#define __S3C_NAND_H

#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/clk.h>

//内核提共, 注册mtd设备，添加分区
int add_mtd_partitions(struct mtd_info *master, const struct mtd_partition *parts, int nbparts);

//描述nand的分区情况
struct nand_part_info {
	struct mtd_partition *partition;
	int num_partitions;
	int num_chips;
};

//描述nand设备
struct nand_info {
	struct nand_chip chip;
    //每一个mtd设备都对应一个mtd_info
	struct mtd_info mtd;
	struct nand_part_info *part;
	void __iomem *virt;
	struct clk *clk;
};

#define S3C_PA_NAND	0x70200000
#define S3C_SZ_NAND	SZ_4K

#define NFCONF          0x0
#define NFCONT          0x4
#define NFCMMD          0x8
#define NFADDR          0xc
#define NFDATA          0x10
#define NFSTAT          0x28

#define NAND_READY	0x1

//NFCONF
#define TACLS		(0 << 12)
#define TWRPH0		(2 << 8)
#define TWRPH1		(1 << 4)
#define MUST1		(1 << 2)

#define CSN2_DIS        (1 << 1)
#define CSN2_ENA        ~(1 << 1)
#define CTROLLER_EN     1

#define RESET_CMD       0xff

#endif
