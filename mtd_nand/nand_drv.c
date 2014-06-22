#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include "s3c_nand.h"

void nand_disable(struct nand_info *nand)
{
	u32 tmp;

	tmp = readl(nand->virt + NFCONT);
	tmp |= CSN2_DIS;
	writel(tmp, nand->virt + NFCONT);
}

void nand_enable(struct nand_info *nand)
{
	u32 tmp;

	tmp = readl(nand->virt + NFCONT);
	tmp &= CSN2_ENA;
	writel(tmp, nand->virt + NFCONT);
}

int nand_dev_ready(struct mtd_info *mtd)
{
	struct nand_info *nand = container_of(mtd, struct nand_info, mtd);

	return readl(nand->virt + NFSTAT) & NAND_READY;
}

void nand_init(struct nand_info *nand)
{
	u32 tmp;
	
    //配置nand时序
	tmp = TACLS | TWRPH0 | TWRPH1 | MUST1;
	writel(tmp, nand->virt + NFCONF);
    //打开nand控制器，并把nand芯片的片选无效
	tmp = CSN2_DIS | CTROLLER_EN;
	writel(tmp, nand->virt + NFCONT);

    //打开nand片选
	nand_enable(nand);

    //发送重启命令
	writel(RESET_CMD, nand->virt + NFCMMD);
	while(!(nand_dev_ready(&nand->mtd)));

    //关掉nand芯片的片选
	nand_disable(nand);
}

void nand_exit(struct nand_info *nand)
{

}

//mtd子系统有对nand的请求时候调用
void nand_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	struct nand_info *nand = container_of(mtd, struct nand_info, mtd);	

	if(ctrl & NAND_CTRL_CHANGE){
		if(ctrl & NAND_NCE)
			nand_enable(nand);
		else
			nand_disable(nand);
	}

	if(dat != NAND_CMD_NONE){
		if(ctrl & NAND_CLE)
			writeb(dat, nand->virt + NFCMMD);
		else if(ctrl & NAND_ALE)
			writeb(dat, nand->virt + NFADDR);
	}
}

int nprobe(struct platform_device *pdev)
{
	struct nand_info *nand;
	int ret = 0;

	struct resource *io_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!io_res)
		return -EBUSY;

	nand = kzalloc(sizeof(*nand), GFP_KERNEL);
	if(!nand)
		return -ENOMEM;

	nand->virt = ioremap(io_res->start, io_res->end - io_res->start + 1);	
	if(!nand->virt){
		ret = -ENOMEM;
		goto ioremap_error;
	}
	
	nand->part = pdev->dev.platform_data;

	nand->chip.IO_ADDR_R = nand->virt + NFDATA;	
	nand->chip.IO_ADDR_W = nand->virt + NFDATA;	

	nand->chip.dev_ready = nand_dev_ready;
	nand->chip.cmd_ctrl = nand_cmd_ctrl;
	
	nand->chip.ecc.mode = NAND_ECC_SOFT;
	//nand->options = NAND_SKIP_BBTSCAN,

	nand->mtd.priv = &nand->chip;
	nand->mtd.owner = THIS_MODULE;
	
    //设置nand子系统中的默认的操作nand的方法
	ret = nand_scan(&nand->mtd, nand->part->num_chips);
	if(ret)
		goto nand_scan_error;
			
	
    //注册mtd设备，添加分区
	ret = add_mtd_partitions(&nand->mtd, nand->part->partition, nand->part->num_partitions);
	if(ret)
		goto add_part_error;

	platform_set_drvdata(pdev, nand);

	nand->clk = clk_get(NULL, "nand");
	clk_enable(nand->clk); 

	nand_init(nand);

	return 0;

add_part_error:
nand_scan_error:
	iounmap(nand->virt);
ioremap_error:
	kfree(nand);
	return ret;
}

int nremove(struct platform_device *pdev)
{
	struct nand_info *nand = platform_get_drvdata(pdev);	
	
	nand_exit(nand);
	clk_disable(nand->clk);
	clk_put(nand->clk);
	nand_release(&nand->mtd);
	iounmap(nand->virt);
	kfree(nand);
	
	return 0;
}

void nshutdown(struct platform_device *pdev)
{

}

int nsuspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

int nresume(struct platform_device *pdev)
{
	return 0;
}

struct platform_driver drv = {
	.probe = nprobe,
	.remove = nremove,
	.suspend = nsuspend,
	.shutdown = nshutdown,
	.resume = nresume,
	.driver = {
		.name = "s3c-nand-flash",
	},
};

static __init int nand_test_init(void)
{
	return platform_driver_register(&drv);
}

static __exit void nand_test_exit(void)
{
	platform_driver_unregister(&drv);
}

module_init(nand_test_init);
module_exit(nand_test_exit);

MODULE_LICENSE("GPL");
