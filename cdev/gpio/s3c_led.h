#ifndef __S3C_LED_H
#define __S3C_LED_H

struct led_st {
	int user;
	int flag;
	dev_t no;

	void __iomem *v;

	struct file_operations ops;

	struct cdev dev;
	
	void (*on)(struct led_st *l, int no);
	void (*off)(struct led_st *l, int no);
};

#define S3C_PA_LED	0x7f008000
#define LEDCON		0x820
#define LEDDAT		0x824
#define S3C_SZ_LED	SZ_4K

#endif
