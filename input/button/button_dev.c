#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include "s3c_button.h"

int kv[6] = {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_ENTER,  KEY_ESC};

struct key_platform_data key_data = {
	.key = kv,	
	.num = 6,	
	.info = "Here is key_value",
	.flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
};

void btn_release(struct device *dev)
{

}

//经过查看电路图得知：六个按键分别对应的是外部中断0-5
//并且按下按键产生下降沿，反之上升沿
struct resource btn_res[] = {
	[0] = {
		.start = S3C_PA_BUTTON,
		.end = S3C_PA_BUTTON + S3C_SZ_BUTTON - 1,
		.name = "s3c-button",
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_EINT(0),
		.end = IRQ_EINT(5),
		.name = "button-irq",
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device dev = {
	.id = -1,
	.name = "s3c-button",
	.num_resources = ARRAY_SIZE(btn_res),
	.resource = btn_res,
	.dev = {
		.release = btn_release,
		.platform_data = &key_data,
	}
};

static __init int button_test_init(void)
{
	return platform_device_register(&dev);
}

static __exit void button_test_exit(void)
{
	platform_device_unregister(&dev);
}

module_init(button_test_init);
module_exit(button_test_exit);

MODULE_LICENSE("GPL");
