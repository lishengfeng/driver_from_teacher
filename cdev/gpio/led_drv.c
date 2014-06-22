#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include "s3c_led.h"

void s3c_led_on(struct led_st *l, int no)
{
	u32 tmp;
	
	tmp = ioread32(l->v + LEDDAT);
	tmp &= ~(1 << no);
	iowrite32(tmp, l->v + LEDDAT);
}

void s3c_led_off(struct led_st *l, int no)
{
	u32 tmp;
	
	tmp = ioread32(l->v + LEDDAT);
	tmp |= (1 << no);
	iowrite32(tmp, l->v + LEDDAT);
}

void s3c_led_init(struct led_st *l)
{
	u32 tmp;

	tmp = ioread32(l->v + LEDCON);
	tmp &= ~0xffff;
	tmp |= 0x1111;
	iowrite32(tmp, l->v + LEDCON);
		
	tmp = ioread32(l->v + LEDDAT);
	tmp |= 0xf;
	iowrite32(tmp, l->v + LEDDAT);
}

void s3c_led_exit(struct led_st *l)
{
	u32 tmp;

	tmp = ioread32(l->v + LEDDAT);
        tmp |= 0xf;
        iowrite32(tmp, l->v + LEDDAT);
}

//-----------------------------------------------
ssize_t lread(struct file *fp, char __user *buffer, size_t count, loff_t *offset)
{
	char buf[64];
	int ret = 0;
	int ret1;

	struct led_st *l = fp->private_data;
	
	ret += sprintf(buf, "state:%d\n", l->flag);

	ret1 = copy_to_user(buffer, buf, ret);
	if(ret1)
		return -EFAULT;
	
	return ret;
}

//int ret = write(fd, "21", 5);
ssize_t lwrite(struct file *fp, const char __user *buffer, size_t count, loff_t *offset)
{
	struct led_st *l = fp->private_data;
	
	if(buffer[1] - '0'){
		s3c_led_on(l, buffer[0] - '0');
		l->flag |= 1 << (buffer[0] - '0');
	}else{
		s3c_led_off(l, buffer[0] - '0');
		l->flag &= ~(1 << (buffer[0] - '0'));
	}

	return count;
}

int lopen(struct inode *no, struct file *fp)
{
	struct led_st *l = container_of(no->i_cdev, struct led_st, dev);

	if(!l->user)
		l->user++;
	else
		return -EBUSY;

	fp->private_data = l;

        return 0;
}

int lclose(struct inode *no, struct file *fp)
{
	struct led_st *l = fp->private_data;

	if(l->user)
		l->user--;
	else
		return -ENODEV;

        return 0;
}

struct led_st *led;

static __init int led_test_init(void)
{
	int ret;

	led = kzalloc(sizeof(struct led_st), GFP_KERNEL);
	if(!led)
		return -ENOMEM;
	ret = alloc_chrdev_region(&led->no, 0, 1, "led");
	if(ret)
		goto alloc_no_error;
	
	led->ops.open = lopen;
	led->ops.release = lclose;
	led->ops.read = lread;
	led->ops.write = lwrite;

	cdev_init(&led->dev, &led->ops);
	ret = cdev_add(&led->dev, led->no, 1);
	if(ret)
		goto add_dev_error;

	//0x7f008000
	led->v = ioremap(S3C_PA_LED, S3C_SZ_LED);
	if(!led->v){
		ret = -ENOMEM;
		goto remap_error;
	}

	s3c_led_init(led);

	return 0;
remap_error:
	cdev_del(&led->dev);
add_dev_error:
	unregister_chrdev_region(led->no, 1);
alloc_no_error:
	kfree(led);
	return ret;
}

static __exit void led_test_exit(void)
{
	s3c_led_exit(led);
	iounmap(led->v);
	cdev_del(&led->dev);
	unregister_chrdev_region(led->no, 1);
        kfree(led);
}

module_init(led_test_init);
module_exit(led_test_exit);

MODULE_LICENSE("GPL");
