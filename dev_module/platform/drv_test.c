#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include "s3c_led.h"

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
//---------------------------------------------

struct file_operations led_ops = {
	.open = lopen,
	.release = lclose,
	.write = lwrite,
	.read = lread,	
};

//设备和驱动匹配成功调用，把和该驱动匹配成功的设备传进来
int led_probe(struct platform_device *pdev)
{
	struct resource *l_res;
	struct led_dev *led;
	int ret;
	struct led_colour *lc = pdev->dev.platform_data;
	
	l_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!l_res)
		return -EBUSY;

	led = kzalloc(sizeof(struct led_dev), GFP_KERNEL);
	if(!led)
		return -ENOMEM;

	led->v = ioremap(l_res->start, l_res->end - l_res->start + 1);
	if(!led->v){
		ret = -ENOMEM;
		goto remap_error;
	}

	ret = alloc_chrdev_region(&led->no, 0, 1, pdev->name);
	if(ret)
		goto alloc_no_error;

	cdev_init(&led->cdev, &led_ops);

	ret = cdev_add(&led->cdev, led->no, 1);
	if(ret)
		goto add_error;

	s3c_led_init(led);

	platform_set_drvdata(pdev, led);
		
	printk("Led with %s is OK!!!\n", lc->colour);	

	return 0;
add_error:
	unregister_chrdev_region(led->no, 1);
alloc_no_error:
	iounmap(led->v);
remap_error:
	kfree(led);
	return ret;
}

//设备或者驱动注销的时候调用
int led_remove(struct platform_device *pdev)
{
	struct led_dev *led = platform_get_drvdata(pdev);

	s3c_led_exit(led);
	cdev_del(&led->cdev);
	unregister_chrdev_region(led->no, 1);
	iounmap(led->v);
	kfree(led);

	return 0;
}

//关机的时候调用
void led_shutdown(struct platform_device *pdev)
{

}

//睡眠时候调用
int led_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

//从睡眠中回复的时候调
int led_resume(struct platform_device *pdev)
{
	return 0;
}

struct platform_driver drv = {
	.driver = {
		.name = "s3c-led",
	},
	.probe = led_probe,
	.remove = led_remove,
	.shutdown = led_shutdown,
	.suspend = led_suspend, 
	.resume = led_resume, 
};

static __init int drv_test_init(void)
{
	return platform_driver_register(&drv);
}

static __exit void drv_test_exit(void)
{
	platform_driver_unregister(&drv);
}

module_init(drv_test_init);
module_exit(drv_test_exit);

MODULE_LICENSE("GPL");





