#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
	struct sched_param param;
	int wdt;

	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	
	sched_setscheduler(0, SCHED_FIFO, &param);

	wdt = open("wdt", O_RDWR);	
	if(wdt < 0){
		perror("open wdt");
		exit(1);
	}

	//for ioctl
	#define WDT_ENABLE              (1)
	#define WDT_DISABLE             (2)
	#define WDT_FEED                (3)
	#define WDT_READ                (4)
	#define WDT_SELECT_RESET        (5)
	#define WDT_SELECT_IRQ          (6)

//	ioctl(wdt, WDT_SELECT_RESET, 0);
	ioctl(wdt, WDT_SELECT_IRQ, 0);
	ioctl(wdt, WDT_ENABLE, 0);
	
//	while(1){
//		ioctl(wdt, WDT_FEED, 15625 * 4);
//		sleep(3);
//	}
	sleep(100);

	ioctl(wdt, WDT_DISABLE, 0);
	close(wdt);

	return 0;
}






