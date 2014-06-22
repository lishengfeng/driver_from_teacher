#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>

struct cdev dev;
dev_t no;
//read(fd, buf, len)
//buf--->buffer
//len--->count
ssize_t mread(struct file *fp, char __user *buffer, size_t count, loff_t *offset)
{
	printk("read\n");

	return 0;
}
//write(fd, buf, len)
//buf--->buffer
//len--->count
ssize_t mwrite(struct file *fp, const char __user *buffer, size_t count, loff_t *offset)
{
	printk("write\n");

	return 0;
}

//open("xxx", xxx)
int mopen(struct inode *no, struct file *fp)
{
	printk("open\n");
	
	return 0;
}
//close(xx)
int mclose(struct inode *no, struct file *fp)
{
	printk("close\n");

	return 0;
}

struct file_operations ops = {
	.open = mopen,
	.release = mclose,
	.read = mread,
	.write = mwrite,
};

static __init int cdev_test_init(void)
{
	int ret;

	ret = alloc_chrdev_region(&no, 0, 1, "mdev");
	if(ret < 0)
		return ret;

	cdev_init(&dev, &ops);	
	ret = cdev_add(&dev, no, 1);
	if(ret < 0)
		goto cdev_add_error;

	printk("M:%d  N:%d\n", MAJOR(no), MINOR(no));

	return 0;
cdev_add_error:
	unregister_chrdev_region(no, 1);
	return ret;
}

static __exit void cdev_test_exit(void)
{
	cdev_del(&dev);
	unregister_chrdev_region(no, 1);
}

module_init(cdev_test_init);
module_exit(cdev_test_exit);

MODULE_LICENSE("GPL");
