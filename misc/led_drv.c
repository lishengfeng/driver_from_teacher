#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "s3c_led.h"

void s3c_led_on(struct led_info *l, int no)
{
        u32 tmp;

        tmp = ioread32(l->v + LEDDAT);
        tmp &= ~(1 << no);
        iowrite32(tmp, l->v + LEDDAT);
}

void s3c_led_off(struct led_info *l, int no)
{
        u32 tmp;

        tmp = ioread32(l->v + LEDDAT);
        tmp |= (1 << no);
        iowrite32(tmp, l->v + LEDDAT);
}

void s3c_led_init(struct led_info *l)
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

void s3c_led_exit(struct led_info *l)
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

        struct led_info *l = fp->private_data;

        ret += sprintf(buf, "state:%d\n", l->flag);

        ret1 = copy_to_user(buffer, buf, ret);
        if(ret1)
                return -EFAULT;

        return ret;
}
//int ret = write(fd, "21", 5);
ssize_t lwrite(struct file *fp, const char __user *buffer, size_t count, loff_t *offset)
{
        struct led_info *l = fp->private_data;

        if(buffer[1] - '0'){
                s3c_led_on(l, buffer[0] - '0');
                l->flag |= 1 << (buffer[0] - '0');
        }else{
                s3c_led_off(l, buffer[0] - '0');
                l->flag &= ~(1 << (buffer[0] - '0'));
        }

        return count;
}

//open(misc)-->sys_open inode  file
int lopen(struct inode *no, struct file *fp)
{
        struct led_info *l = container_of(fp->f_op, struct led_info, led_ops);

        if(!l->user)
                l->user++;
        else
                return -EBUSY;

        fp->private_data = l;

        return 0;
}

int lclose(struct inode *no, struct file *fp)
{
        struct led_info *l = fp->private_data;

        if(l->user)
                l->user--;
        else
                return -ENODEV;

        return 0;
}

//---------------------------------------------

int led_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *led_res;
	struct led_platform_data *led_data;
	struct led_info *led;

	led_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!led_res)
		return -EBUSY;
	
	led_data = pdev->dev.platform_data;

	led = kzalloc(sizeof(struct led_info), GFP_KERNEL);
	if(!led)
		return -ENOMEM;

	led->v = ioremap(led_res->start, led_res->end - led_res->start + 1);
	if(!led->v){
		ret = -ENOMEM;
		goto remap_error;
	}

	led->misc.minor = MISC_DYNAMIC_MINOR;
	led->misc.name = pdev->name;

        led->led_ops.open = lopen;
        led->led_ops.release = lclose;
        led->led_ops.write = lwrite;
        led->led_ops.read = lread;

	led->misc.fops = &led->led_ops;	
	
	ret = misc_register(&led->misc);
	if(ret)
		goto misc_register_error;
		
	s3c_led_init(led);

	platform_set_drvdata(pdev, led);

	printk("LED DRIVER:%s\n", led_data->info);

	return 0;
misc_register_error:
	iounmap(led->v);
remap_error:
	kfree(led);
	return ret;
}

int led_remove(struct platform_device *pdev)
{
	struct led_info *led = platform_get_drvdata(pdev);		
	s3c_led_exit(led);
	misc_deregister(&led->misc);	
	iounmap(led->v);
	kfree(led);
	
	return 0;
}

void led_shutdown(struct platform_device *pdev)
{
	struct led_info *led = platform_get_drvdata(pdev);		
	s3c_led_exit(led);	
}

int led_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct led_info *led = platform_get_drvdata(pdev);		
	led->ledcon = ioread32(led->v + LEDCON);
	led->leddat = ioread32(led->v + LEDDAT);

	s3c_led_exit(led);

	return 0;
}

int led_resume(struct platform_device *pdev)
{
	struct led_info *led = platform_get_drvdata(pdev);		
	iowrite32(led->ledcon, led->v + LEDCON);
	iowrite32(led->leddat, led->v + LEDDAT);

	return 0;
}

struct platform_driver drv = {
	.driver = {
		.name = "myled",
	},
	.probe = led_probe,
	.remove = led_remove,
	.suspend = led_suspend,
	.shutdown = led_shutdown,
	.resume = led_resume,
};

static __init int led_test_init(void)
{
	return platform_driver_register(&drv);
}

static __exit void led_test_exit(void)
{
	platform_driver_unregister(&drv);
}

module_init(led_test_init);
module_exit(led_test_exit);

MODULE_LICENSE("GPL");
