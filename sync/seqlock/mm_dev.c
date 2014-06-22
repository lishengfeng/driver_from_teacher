#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/delay.h>

struct mm_info {
	atomic_t user;	
	dev_t no;
	char *content;
	int len;
	struct cdev dev;
	seqlock_t lock;
};

ssize_t mm_read(struct file *fp, char __user *buffer, size_t count, loff_t *offset)
{
	int ret;
	int len;
	int start;
	struct mm_info *m = fp->private_data;

	do{
		start = read_seqbegin(&m->lock);	
		len = min(m->len, (int)count);
		ret = copy_to_user(buffer, m->content, len);
		if(ret){
			return -EFAULT;	
		}
	}while(read_seqretry(&m->lock, start));

	return len;
}

ssize_t mm_write(struct file *fp, const char __user *buffer, size_t count, loff_t *offset)
{
	int ret, len;
	struct mm_info *m = fp->private_data;

	write_seqlock(&m->lock);
	len = min((int)count, m->len);
	ret = copy_from_user(m->content, buffer, len);
	if(ret){
		write_sequnlock(&m->lock);
		return -EFAULT;
	}

	write_sequnlock(&m->lock);
	
	return len;
}

int mm_open(struct inode *no, struct file *fp)
{
	struct mm_info *m = container_of(no->i_cdev, struct mm_info, dev);
	fp->private_data = m;

	atomic_inc(&m->user);

	return 0;
}

int mm_release(struct inode *no, struct file *fp)
{
	struct mm_info *m = container_of(no->i_cdev, struct mm_info, dev);

	atomic_dec(&m->user);

	return 0;
}

struct file_operations mm_ops = {
	.open = mm_open,
	.release = mm_release,
	.read = mm_read,
	.write = mm_write,
};

//--------------------------------------
struct mm_info *mm;

static __init int mm_test_init(void)
{
	int ret;

	mm = kzalloc(sizeof(struct mm_info), GFP_KERNEL);
	if(!mm)
		return -ENOMEM;	

	mm->content = kzalloc(0x1000, GFP_KERNEL);	
	if(!mm->content){
		ret = -ENOMEM;
		goto alloc_content_error;
	}
	
	mm->len = 0x1000;

	ret = alloc_chrdev_region(&mm->no, 0, 1, "mm_dev");
	if(ret)
		goto alloc_no_error;

	cdev_init(&mm->dev, &mm_ops);

	ret = cdev_add(&mm->dev, mm->no, 1);
	if(ret)
		goto cdev_add_error;

	seqlock_init(&mm->lock);
	
	atomic_set(&mm->user, 0);

	return 0;
cdev_add_error:
	unregister_chrdev_region(mm->no, 1);
alloc_no_error:
	kfree(mm->content);
alloc_content_error:
	kfree(mm);
	return ret;
}

static __exit void mm_test_exit(void)
{
	cdev_del(&mm->dev);	
	unregister_chrdev_region(mm->no, 1);
	kfree(mm->content);
	kfree(mm);
}

module_init(mm_test_init);
module_exit(mm_test_exit);

MODULE_LICENSE("GPL");
