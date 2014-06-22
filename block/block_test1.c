#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/blkdev.h>

#include "s3c_block.h"

struct ramdisk_st *ramdisk;

static __init int block_test_init(void)
{
	int ret;

	ramdisk = kzalloc(sizeof(*ramdisk), GFP_KERNEL);	
	if(!ramdisk){
		ret = -ENOMEM;
		goto alloc_ramdisk_error;
	}
	//为块设备申请一个主设备号
	//第一参数：0代表让内核分配一个主设备号
	//类似alloc_chrdev_region()
	ramdisk->major = register_blkdev(0, "my_ramdisk");
	if(ramdisk->major < 0){
		ret = ramdisk->major;
		goto register_blkdev_error;
	}

	return 0;
register_blkdev_error:
	kfree(ramdisk);
alloc_ramdisk_error:
	return ret;
}

static __exit void block_test_exit(void)
{
	unregister_blkdev(ramdisk->major, "my_ramdisk");
	kfree(ramdisk);	
}

module_init(block_test_init);
module_exit(block_test_exit);

MODULE_LICENSE("GPL");
