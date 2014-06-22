#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include "s3c_led.h"

extern struct led_bus lbus;

void s3c_led_on(struct led_dev *l, int no)
{
        u32 tmp;

        tmp = ioread32(l->v + LEDDAT);
        tmp &= ~(1 << no);
        iowrite32(tmp, l->v + LEDDAT);
}

void s3c_led_off(struct led_dev *l, int no)
{
        u32 tmp;

        tmp = ioread32(l->v + LEDDAT);
        tmp |= (1 << no);
        iowrite32(tmp, l->v + LEDDAT);
}

void s3c_led_init(struct led_dev *l)
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

void s3c_led_exit(struct led_dev *l)
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

        struct led_dev *l = fp->private_data;

        ret += sprintf(buf, "state:%d\n", l->flag);

        ret1 = copy_to_user(buffer, buf, ret);
        if(ret1)
                return -EFAULT;

        return ret;
}

//int ret = write(fd, "21", 5);
ssize_t lwrite(struct file *fp, const char __user *buffer, size_t count, loff_t *offset)
{
        struct led_dev *l = fp->private_data;

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
        struct led_dev *l = container_of(no->i_cdev, struct led_dev, cdev);

        if(!l->user)
                l->user++;
        else
                return -EBUSY;

        fp->private_data = l;

        return 0;
}

int lclose(struct inode *no, struct file *fp)
{
        struct led_dev *l = fp->private_data;

        if(l->user)
                l->user--;
        else
                return -ENODEV;

        return 0;
}

struct file_operations led_ops = {
	.open = lopen,
	.release = lclose,
	.write = lwrite,
	.read = lread,	
};

//设备和驱动匹配成功调用，把和该驱动匹配成功的设备传进来
int led_probe(struct device *dev)
{
	int ret;

	struct led_dev *ldev = container_of(dev, struct led_dev, dev);

	printk("Device %s probe\n", ldev->colour);

	ldev->flag = 0;
	ldev->user = 0;

	ret = alloc_chrdev_region(&ldev->no, 0, 1, ldev->colour);
	if(ret < 0)
		return ret;
	cdev_init(&ldev->cdev, &led_ops);
	ret = cdev_add(&ldev->cdev, ldev->no, 1);
	if(ret < 0)
		goto cdev_add_error;
	
	ldev->v = ioremap(S3C_PA_LED, S3C_SZ_LED);
	if(!ldev->v){
		ret = -ENOMEM;
		goto ioremap_error;	
	}

	s3c_led_init(ldev);

	return 0;
ioremap_error:
	cdev_del(&ldev->cdev);
cdev_add_error:
	unregister_chrdev_region(ldev->no, 1);
	return ret;
}

//设备或者驱动注销的时候调用
int led_remove(struct device *dev)
{
	struct led_dev *ldev = container_of(dev, struct led_dev, dev);

	printk("remove\n");

	s3c_led_exit(ldev);
	iounmap(ldev->v);
	cdev_del(&ldev->cdev);
	unregister_chrdev_region(ldev->no, 1);

	return 0;
}

//关机的时候调用
void led_shutdown(struct device *dev)
{

}

//睡眠时候调用
int led_suspend(struct device *dev, pm_message_t state)
{
	return 0;
}

//从睡眠中回复的时候调
int led_resume(struct device *dev)
{
	return 0;
}

struct led_drv ldrv = {
	.colour = "red",
	.drv = {
		.name = "red",
		.bus = &lbus.bus,
		.probe = led_probe,
		.remove = led_remove,
		.shutdown = led_shutdown,
		.suspend = led_suspend, .resume = led_resume, } }; 
static __init int drv_test_init(void)
{
	return driver_register(&ldrv.drv);
}

static __exit void drv_test_exit(void)
{
	driver_unregister(&ldrv.drv);
}

module_init(drv_test_init);
module_exit(drv_test_exit);

MODULE_LICENSE("GPL");





