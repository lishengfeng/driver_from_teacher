#ifndef __S3C_BUTTON_H
#define __S3C_BUTTON_H

struct key_platform_data {
        int *key;
        char *info;
	int num;	
	int flags;
};

struct key_info {
	int irq;
	int code;
	char name[32];
	void __iomem *v;
	
	struct input_dev *dev;
};

#define S3C_PA_BUTTON	0x7F008000
#define S3C_SZ_BUTTON	SZ_4K

#define GPNCON		0x830
#define GPNDAT		0x834

#endif
