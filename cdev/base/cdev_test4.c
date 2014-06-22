#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

struct mdev_st {
	int user;
	dev_t no;
	char *content;
	int len;
	struct cdev dev;
};

ssize_t mread(struct file *fp, char __user *buffer, size_t count, loff_t *offset)
{
	struct mdev_st *m = fp->private_data;
	int ret;

	ret = copy_to_user(buffer, m->content, min(m->len, (int)count));
	if(ret)
		return -EFAULT;

	return min(m->len, (int)count);
}

//int ret = write(fd, "hello", 5);
ssize_t mwrite(struct file *fp, const char __user *buffer, size_t count, loff_t *offset)
{
	struct mdev_st *m = fp->private_data;
	int ret;

	kfree(m->content);
	m->content = kzalloc(count, GFP_KERNEL);
	if(!m->content)	
		return -ENOMEM;
	
	ret = copy_from_user(m->content, buffer, count);
	if(ret){
		kfree(m->content);
		return -EFAULT;
	}
	
	m->len = count;

	return count;
}

int mopen(struct inode *no, struct file *fp)
{
	struct mdev_st *m;

	m = container_of(no->i_cdev, struct mdev_st, dev);
	fp->private_data = m;
	if(!m->user)
		m->user++;
	else
		return -EBUSY;

	return 0;
}

int mclose(struct inode *no, struct file *fp)
{
	struct mdev_st *m = fp->private_data;

	if(m->user)
		m->user--;
	else
		return -ENODEV;	

	return 0;
}

struct file_operations ops = {
	.open = mopen,
	.release = mclose,
	.read = mread,
	.write = mwrite,
};

struct mdev_st *mdev;

static __init int cdev_test_init(void)
{
	int ret;

	mdev = kzalloc(sizeof(struct mdev_st), GFP_KERNEL);
	if(!mdev)
		return -ENOMEM;

	ret = alloc_chrdev_region(&mdev->no, 0, 1, "mdev");
	if(ret < 0)
		goto alloc_dev_error;

	cdev_init(&mdev->dev, &ops);	
	ret = cdev_add(&mdev->dev, mdev->no, 1);
	if(ret < 0)
		goto cdev_add_error;

	printk("M:%d  N:%d\n", MAJOR(mdev->no), MINOR(mdev->no));

	return 0;
cdev_add_error:
	unregister_chrdev_region(mdev->no, 1);
alloc_dev_error:
	kfree(mdev);
	return ret;
}

static __exit void cdev_test_exit(void)
{
	cdev_del(&mdev->dev);
	unregister_chrdev_region(mdev->no, 1);
	kfree(mdev);
}

module_init(cdev_test_init);
module_exit(cdev_test_exit);

MODULE_LICENSE("GPL");
