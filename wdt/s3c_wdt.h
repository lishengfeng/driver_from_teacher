#ifndef __WDT_H
#define __WDT_H

#include <linux/cdev.h>

//for ioctl
#define WDT_ENABLE		(1)
#define WDT_DISABLE		(2)
#define WDT_FEED		(3)
#define WDT_READ		(4)
#define WDT_SELECT_RESET	(5)
#define WDT_SELECT_IRQ		(6)

struct wdt_info {
	int user;
	void __iomem *v;
	
	dev_t no;
	struct cdev dev;
};

#define S3C_PA_WDT	0x7E004000
#define WTCON 		0x0 
#define WTDAT 		0x4 
#define WTCNT 		0x8 
#define WTCLRINT 	0xC 

#endif
