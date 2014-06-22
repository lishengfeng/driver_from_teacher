#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/interrupt.h>

#include "s3c_wdt.h"

void s3c_wdt_init(struct wdt_info *w)
{
	u32 tmp;
		
	//66000000/66/64=15625HZ
	tmp = (65 << 8) | (2 << 3);
	iowrite32(tmp, w->v + WTCON);
	iowrite16(15625 * 4, w->v + WTDAT);
	iowrite16(15625 * 4, w->v + WTCNT);
}

void s3c_wdt_exit(struct wdt_info *w)
{
	u32 tmp;

        tmp = ioread32(w->v + WTCON);
        tmp &= ~(1 << 5);
        iowrite32(tmp, w->v + WTCON);
}

void s3c_wdt_select_reset(struct wdt_info *w, int select)
{
	u32 tmp;
	
	tmp = ioread32(w->v + WTCON);
	tmp &= ~(7);

	if(select){
        	tmp |= 1;
	}else{
		tmp |= 1 << 2;
	}

	iowrite32(tmp, w->v + WTCON);
}

void s3c_wdt_enable(struct wdt_info *w)
{
	u32 tmp;
		
	tmp = ioread32(w->v + WTCON);
	tmp |= 1 << 5;
	iowrite32(tmp, w->v + WTCON);
}

void s3c_wdt_disable(struct wdt_info *w)
{
	u32 tmp;
		
	tmp = ioread32(w->v + WTCON);
	tmp &= ~(1 << 5);
	iowrite32(tmp, w->v + WTCON);
}

void s3c_wdt_feed(struct wdt_info *w, unsigned short food)
{
	iowrite16(food, w->v + WTCNT);
}

void s3c_wdt_read(struct wdt_info *w, unsigned short *v)
{
	*v = ioread16(w->v + WTCNT);
}

void s3c_wdt_clear_irq(struct wdt_info *w)
{
	iowrite32(1, w->v + WTCLRINT); 	
}

//-------------------------------------------
//ioclt(fd, request, cmd);
long wdt_ioctl(struct file *fp, unsigned int request, unsigned long cmd)	
{
	struct wdt_info *w = fp->private_data;

	switch(request){
		case WDT_ENABLE:
			s3c_wdt_enable(w);
			break;
		case WDT_DISABLE:
			s3c_wdt_disable(w);
			break;
		case WDT_FEED:
			s3c_wdt_feed(w, cmd);
			break;
		case WDT_READ:
			s3c_wdt_read(w, (unsigned short *)cmd);
			break;
		case WDT_SELECT_RESET:
			s3c_wdt_select_reset(w, 1);
			break;
		case WDT_SELECT_IRQ:
			s3c_wdt_select_reset(w, 0);
			break;
		default:
			break;
	}

	return 0;
}

int wdt_open(struct inode *no, struct file *fp)
{
	struct wdt_info *w = container_of(no->i_cdev, struct wdt_info, dev);
	fp->private_data = w;

	if(!w->user)
		w->user++;
	else
		return -EBUSY;

	return 0;
}

int wdt_release(struct inode *no, struct file *fp)
{
	struct wdt_info *w = container_of(no->i_cdev, struct wdt_info, dev);

	if(w->user)
		w->user--;
	else
		return -ENODEV;
		
	s3c_wdt_disable(w);

	return 0;
}

ssize_t wdt_read(struct file *fp, char __user *buffer, size_t count, loff_t *offset)
{
	return 0;
}

ssize_t wdt_write(struct file *fp, const char __user *buffer, size_t count, loff_t *offset)
{
	return 0;	
}

struct file_operations wdt_ops = {
	.open = wdt_open,
	.release = wdt_release,
	.read = wdt_read,
	.write = wdt_write,
	.unlocked_ioctl = wdt_ioctl,
};

irqreturn_t do_wdt(int no, void *data)
{
	struct wdt_info *w = data;

	printk("wang wang ....\n");

	s3c_wdt_clear_irq(w);	

	return IRQ_HANDLED;
}

//-----------------------------------------------------
struct wdt_info *wdt;

static __init int wdt_test_init(void)
{
	int ret;

	wdt = kzalloc(sizeof(struct wdt_info), GFP_KERNEL);
	if(!wdt)
		return -ENOMEM;
	
	ret = alloc_chrdev_region(&wdt->no, 0, 1, "mywdt");
	if(ret)
		goto alloc_no_error;

	cdev_init(&wdt->dev, &wdt_ops);
	ret = cdev_add(&wdt->dev, wdt->no, 1);
	if(ret)
		goto cdev_add_error;

	wdt->v = ioremap(S3C_PA_WDT, SZ_4K);
	if(!wdt->v){
		ret = -ENOMEM;
		goto ioremap_error;
	}

	ret = request_irq(IRQ_WDT, do_wdt, 0, "mywdt", wdt);
	if(ret)
		goto request_irq_error;

	s3c_wdt_init(wdt);
	
	return 0;
request_irq_error:
	iounmap(wdt->v);
ioremap_error:
	cdev_del(&wdt->dev);
cdev_add_error:
	unregister_chrdev_region(wdt->no, 1);	
alloc_no_error:
	kfree(wdt);
	return ret;
}

static __exit void wdt_test_exit(void)
{
	s3c_wdt_exit(wdt);
	free_irq(IRQ_WDT, wdt);	
	iounmap(wdt->v);
	cdev_del(&wdt->dev);
	unregister_chrdev_region(wdt->no, 1);	
	kfree(wdt);
}

module_init(wdt_test_init);
module_exit(wdt_test_exit);

MODULE_LICENSE("GPL");
