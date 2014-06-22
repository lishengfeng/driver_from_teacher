#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/input.h>
#include <linux/delay.h>

#include "s3c_button.h"

int s3c_button_is_down(struct key_info *button)
{
	if(!(ioread32(button->v + GPNDAT) & (1 << (button->irq - IRQ_EINT(0))))){
                mdelay(2);
                if(!(ioread32(button->v + GPNDAT) & (1 << (button->irq - IRQ_EINT(0)))))
                        return 1;
                else
                        return 0;
        }else{
                return 0;
        }
}

int s3c_button_is_up(struct key_info *button)
{
	if(ioread32(button->v + GPNDAT) & (1 << (button->irq - IRQ_EINT(0)))){
		mdelay(2);
		if(ioread32(button->v + GPNDAT) & (1 << (button->irq - IRQ_EINT(0))))
			return 1;
		else 
			return 0;
	}else{
		return 0;
	}
}

irqreturn_t button_handler(int no, void *data)
{
	struct key_info *button = data;

	//printk("button %s\n", button->name);
	if(s3c_button_is_down(button))
		input_report_key(button->dev, button->code, 1);
	else if(s3c_button_is_up(button))
		input_report_key(button->dev, button->code, 0);
	else
		return IRQ_NONE;

	input_sync(button->dev);
		
	return IRQ_HANDLED;
}

void s3c_button_init(struct key_info *button, int irq, struct key_platform_data *kdata)
{
	int i;

	for(i = 0; i < kdata->num; i++){
		button[i].irq = i + irq;
		button[i].v = button[0].v;
		button[i].code = kdata->key[i];
		button[i].dev = button[0].dev;
		sprintf(button[i].name, "button%d", i);
	}
}

int btn_probe(struct platform_device *pdev)
{
	int ret;
	int i;
	struct resource *reg_res, *irq_res;
	struct key_platform_data *kdata;
	struct key_info *button;

	printk("Device %s probe\n", pdev->name);

	reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if(!reg_res || !irq_res)
		return -EBUSY;

	kdata = pdev->dev.platform_data;

	printk("data_info:%s\n", kdata->info);
	printk("key_no:%d\n", kdata->num);

	button = kzalloc(sizeof(struct key_info) * kdata->num, GFP_KERNEL);	
	if(!button)
		return -ENOMEM;

	button[0].v = ioremap(reg_res->start, reg_res->end - reg_res->start + 1);
	if(!button[0].v){
		ret = -ENOMEM;
		goto remap_error;
	}

	button[0].dev = input_allocate_device();
	if(!button[0].dev){
		ret = -ENOMEM;
		goto input_alloc_error;
	}

	button[0].dev->name = pdev->name;
        button[0].dev->phys = "/dev/eventx";
        button[0].dev->uniq = "20140411";
        button[0].dev->id.bustype = BUS_HOST;
        button[0].dev->id.vendor = 0x110;
        button[0].dev->id.product = 0x120;
        button[0].dev->id.version = 0x119;

	set_bit(EV_SYN, button[0].dev->evbit);
	set_bit(EV_KEY, button[0].dev->evbit);

	for(i = 0; i < kdata->num; i++)
		set_bit(kdata->key[i], button[0].dev->keybit);

	ret = input_register_device(button[0].dev);
	if(ret)
		goto register_error;

	s3c_button_init(button, irq_res->start, kdata);

	for(i = 0; i < kdata->num; i++){
		ret = request_irq(button[i].irq, button_handler, kdata->flags, button[i].name, &button[i]);
		if(ret)
			goto request_irq_error;
	}

	platform_set_drvdata(pdev, button);

	return 0;
request_irq_error:
	while(i--)
		free_irq(button[i].irq, &button[i]);	
	input_unregister_device(button[0].dev);	
register_error:
	input_free_device(button[0].dev);	
input_alloc_error:
	iounmap(button[0].v);
remap_error:
	kfree(button);	
	return ret;
}

int btn_remove(struct platform_device *pdev)
{
	int i;
	struct key_platform_data *kdata;
        struct key_info *button;

	button = platform_get_drvdata(pdev);
	kdata = pdev->dev.platform_data;
	i = kdata->num;

	while(i--)
                free_irq(button[i].irq, &button[i]);
	input_unregister_device(button[0].dev);
        input_free_device(button[0].dev);
	iounmap(button[0].v);
        kfree(button);

	return 0;
}

struct platform_driver drv = {
	.driver = {
		.name = "s3c-button",
	},
	.probe = btn_probe,
	.remove = btn_remove,
};

static __init int button_test_init(void)
{
	return platform_driver_register(&drv);
}

static __exit void button_test_exit(void)
{
	platform_driver_unregister(&drv);
}

module_init(button_test_init);
module_exit(button_test_exit);

MODULE_LICENSE("GPL");
