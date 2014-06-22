#ifndef __S3C_TS_H
#define __S3C_TS_H

#include <linux/input.h>

struct ts_info {
	int user;
	int irq_pen;
	int irq_adc;
	int count;
	int x;
	int y;
	void __iomem *v;
	
	struct clk *clk;
	
	struct timer_list time;
	
	struct input_dev *dev;
};

#define S3C_PA_TS	0x7E00B000 
#define S3C_SZ_TS	SZ_4K

#define ADCCON 			0x00 
#define ADCTSC 			0x04 
#define ADCDLY 			0x08 
#define ADCDAT0 		0x0C
#define ADCDAT1 		0x10
#define ADCUPDN 		0x14
#define ADCCLRINT 		0x18 
#define ADCCLRINTPNDNUP 	0x20 

#endif









