#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>

dev_t no;

static __init int cdev_test_init(void)
{
	int ret;

	no = MKDEV(250, 10);
	//参数：设备号  要注册的设备号个数  设备名字
	ret = register_chrdev_region(no, 2, "mycdev");
	if(ret < 0)
		return ret;

	return 0;	
}

static __exit void cdev_test_exit(void)
{
	unregister_chrdev_region(no, 2);
}

module_init(cdev_test_init);
module_exit(cdev_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZhiYong Li");
MODULE_VERSION("2.0");
MODULE_DESCRIPTION("Cdev for Class");





