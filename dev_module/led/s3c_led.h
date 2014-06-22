#ifndef __S3C_LED_H
#define __S3C_LED_H

#include <linux/cdev.h>
#include <linux/device.h>

#define NAME_SIZE	32

struct led_bus {
	char *name;
	struct bus_type bus;
};

struct led_dev {
	char *colour;
	int user;
	dev_t no;
	void __iomem *v;
	int flag;
	struct cdev cdev;
	struct device dev;
};

struct led_drv {
	char *colour;	
	struct device_driver drv;
};

#define S3C_PA_LED	0x7f008000
#define S3C_SZ_LED	SZ_4K
#define LEDCON		0x820
#define LEDDAT		0x824

#endif





