#ifndef __S3C_LED_H
#define __S3C_LED_H

#include <linux/miscdevice.h>
#include <linux/fs.h>

struct led_platform_data {
	char *info;
};

struct led_info {
	int user;
	int flag;
	int ledcon;
	int leddat;

	struct file_operations led_ops;

	void __iomem *v;
	
	struct miscdevice misc;
};

#define S3C_PA_LED	0x7f008000
#define S3C_SZ_LED	SZ_4K

#define LEDCON		0x820
#define LEDDAT		0x824

#endif






