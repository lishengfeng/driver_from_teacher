#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>

dev_t no;

static __init int cdev_test_init(void)
{
	int ret;

	//参数：要存放设备号的地址  第一个次设备号  要申请的设备号的个数  该设备的名字
	ret = alloc_chrdev_region(&no, 2, 3, "hello");
	if(ret < 0)
		return ret;
	
	printk("Major:%d  Minor:%d\n", MAJOR(no), MINOR(no));

	return 0;
}

static __exit void cdev_test_exit(void)
{
	unregister_chrdev_region(no, 3);
}

module_init(cdev_test_init);
module_exit(cdev_test_exit);

MODULE_LICENSE("GPL");



