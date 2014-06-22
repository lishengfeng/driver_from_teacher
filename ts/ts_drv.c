#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/interrupt.h>

#include "s3c_ts.h"

void s3c_adc_init(struct ts_info *ts)
{
	//3M
	u32 tmp = (1 << 16) | (1 << 14) | (21 << 6);
	iowrite32(tmp, ts->v + ADCCON);
	iowrite32(0xffff, ts->v + ADCDLY);
}

void s3c_ts_set_wait_down(struct ts_info *ts)
{
	u32 tmp = 0xd3;
	iowrite32(tmp, ts->v + ADCTSC);
}

void s3c_ts_set_wait_up(struct ts_info *ts)
{
	u32 tmp = 0xd3 | (1 << 8);
	iowrite32(tmp, ts->v + ADCTSC);
}

void s3c_ts_set_auto(struct ts_info *ts)
{
	iowrite32((1 << 2), ts->v + ADCTSC);
}

void s3c_start_adc(struct ts_info *ts)
{
	u32 tmp;

	tmp = ioread32(ts->v + ADCCON); 
	tmp |= 1;
	iowrite32(tmp, ts->v + ADCCON);
}

void s3c_clear_adc_irq(struct ts_info *ts)
{
	iowrite32(1, ts->v + ADCCLRINT);
}

void s3c_clear_pen_irq(struct ts_info *ts)
{
	iowrite32(1, ts->v + ADCCLRINTPNDNUP);
}

int s3c_if_down(struct ts_info *ts)
{
	return ioread32(ts->v + ADCUPDN) & 1;
}

int s3c_if_up(struct ts_info *ts)
{
	return ioread32(ts->v + ADCUPDN) & (1 << 1);
}

void s3c_ts_init(struct ts_info *ts)
{
	s3c_adc_init(ts);
	s3c_ts_set_wait_down(ts);
}

int s3c_ts_get_x(struct ts_info *ts)
{
	return ioread32(ts->v + ADCDAT0) & 0xfff;
}

int s3c_ts_get_y(struct ts_info *ts)
{
	return ioread32(ts->v + ADCDAT1) & 0xfff;
}

void s3c_clear_down(struct ts_info *ts)
{
	u32 tmp;

	tmp = ioread32(ts->v + ADCUPDN);
	tmp &= ~1;
	iowrite32(tmp, ts->v + ADCUPDN);
}

void s3c_clear_up(struct ts_info *ts)
{
	u32 tmp;

        tmp = ioread32(ts->v + ADCUPDN);
        tmp &= ~(1 << 1);
        iowrite32(tmp, ts->v + ADCUPDN);
}

irqreturn_t s3c_do_adc(int no, void *data)
{
	struct ts_info *ts = data;

	//printk("x:%d\ny:%d\n", s3c_ts_get_x(ts), s3c_ts_get_y(ts));
	ts->count++;
	ts->x += s3c_ts_get_x(ts);
	ts->y += s3c_ts_get_y(ts);
	if(ts->count == 5){
		input_report_abs(ts->dev, ABS_X, ts->x);
		input_report_abs(ts->dev, ABS_Y, ts->y);
		input_report_key(ts->dev, BTN_TOUCH, 1);
		input_sync(ts->dev);
	
		ts->x = 0;
		ts->y = 0;
		ts->count = 0;
	}
	
	s3c_clear_adc_irq(ts);
	s3c_ts_set_wait_up(ts);

	return IRQ_HANDLED;
}

void do_time(unsigned long data)
{
	struct ts_info *ts = (void *)data;

	//设置转换方式：设置为自动转换
	s3c_ts_set_auto(ts);//ADCTSC = 1 << 2
	s3c_start_adc(ts);		

	mod_timer(&ts->time, jiffies + 1);
}

irqreturn_t s3c_do_updn(int no, void *data)
{
	struct ts_info *ts = data;

//	printk("hello pen\n");
	
	if(s3c_if_down(ts)){
		s3c_clear_down(ts);	
//		printk("down\n");
		mod_timer(&ts->time, jiffies + 1);
	}else if(s3c_if_up(ts)){
		s3c_clear_up(ts);
		del_timer(&ts->time);
//		printk("up\n");
		input_report_key(ts->dev, BTN_TOUCH, 0);
		input_sync(ts->dev);
		s3c_ts_set_wait_down(ts);
	}else{
		return IRQ_NONE;
	}

	s3c_clear_pen_irq(ts);	

	return IRQ_HANDLED;
}

//---------------------------------------------------

int ts_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *reg_res, *irq_res;
	struct ts_info *ts;
	
	printk("Device %s probe\n", pdev->name);

	reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if(!reg_res || !irq_res)
		return -EBUSY;

	ts = kzalloc(sizeof(struct ts_info), GFP_KERNEL);
	if(!ts)
		return -ENOMEM;	

	ts->irq_pen = irq_res->start;
	ts->irq_adc = irq_res->end;

	ts->dev = input_allocate_device();
	if(!ts->dev){
		ret = -ENOMEM;
		goto alloc_dev_error;
	}

	ts->dev->name = pdev->name;
        ts->dev->phys = "/dev/eventx";
        ts->dev->uniq = "20140412";
        ts->dev->id.bustype = BUS_HOST;
        ts->dev->id.vendor = 0x110;
        ts->dev->id.product = 0x120;
        ts->dev->id.version = 0x130;

	set_bit(EV_SYN, ts->dev->evbit);
	set_bit(EV_KEY, ts->dev->evbit);
	set_bit(EV_ABS, ts->dev->evbit);

	set_bit(BTN_TOUCH, ts->dev->keybit);

	set_bit(ABS_X, ts->dev->absbit);
	set_bit(ABS_Y, ts->dev->absbit);

	//3.x内核
	input_set_abs_params(ts->dev, ABS_X, 0, 0xfff, 0, 0);
	input_set_abs_params(ts->dev, ABS_Y, 0, 0xfff, 0, 0);

	ret = input_register_device(ts->dev);	
	if(ret)
		goto register_dev_error;

	ts->v = ioremap(reg_res->start, reg_res->end - reg_res->start + 1);
	if(!ts->v){
		ret = -ENOMEM;
		goto remap_error;
	}	

	//要打开adc的时钟
	//arch/arm/mach-s3c64xx/clock.c
	ts->clk = clk_get(NULL, "adc");
	if(PTR_RET(ts->clk)){
		ret = PTR_RET(ts->clk);
		goto clk_get_error;	
	}

	ret = clk_enable(ts->clk);
	if(ret)
		goto clk_enable_error;

	ret = request_irq(ts->irq_adc, s3c_do_adc, 0, "s3c-ts-adc", ts);
	if(ret)
		goto request_adc_error;	

	ret = request_irq(ts->irq_pen, s3c_do_updn, 0, "s3c-ts-pen", ts);
	if(ret)
		goto request_pen_error;
	
	setup_timer(&ts->time, do_time, (unsigned long)ts);

	s3c_ts_init(ts);

	platform_set_drvdata(pdev, ts);
	
	return 0;
request_pen_error:
	free_irq(ts->irq_adc, ts);
request_adc_error:
	clk_disable(ts->clk);
clk_enable_error:
	clk_put(ts->clk);
clk_get_error:
	iounmap(ts->v);
remap_error:
	input_unregister_device(ts->dev);	
register_dev_error:
	input_free_device(ts->dev);	
alloc_dev_error:
	kfree(ts);
	return ret;
}

int ts_remove(struct platform_device *pdev)
{
	struct ts_info *ts;

	ts = platform_get_drvdata(pdev);
	free_irq(ts->irq_adc, ts);
	free_irq(ts->irq_pen, ts);
        clk_disable(ts->clk);
        clk_put(ts->clk);
        iounmap(ts->v);
	input_unregister_device(ts->dev);	
	input_free_device(ts->dev);	
	kfree(ts);

	return 0;
}

struct platform_driver drv = {
	.driver = {
		.name = "s3c-ts",
	},
	.probe = ts_probe,
	.remove = ts_remove,
};

static __init int ts_test_init(void)
{
	return platform_driver_register(&drv);
}

static __exit void ts_test_exit(void)
{
	platform_driver_unregister(&drv);
}

module_init(ts_test_init);
module_exit(ts_test_exit);

MODULE_LICENSE("GPL");




