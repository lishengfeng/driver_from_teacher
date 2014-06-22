#ifndef __S3C_LED_H
#define __S3C_LED_H

#include <linux/cdev.h>
#include <linux/device.h>

struct led_colour {
        char *colour;
};

struct led_dev {
	char *colour;
	int user;
	dev_t no;
	void __iomem *v;
	int flag;
	struct class *led_class;
	struct device *led_class_dev;
	struct cdev cdev;
};

#define S3C_PA_LED	0x7f008000
#define S3C_SZ_LED	SZ_4K
#define LEDCON		0x820
#define LEDDAT		0x824

#endif





