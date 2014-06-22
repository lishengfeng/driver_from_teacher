#include <stdio.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

/*
查看当前系统中的输入设备有哪些？
cat /proc/bus/input/devices

*/
// ./a.out /dev/event0
int main(int argc, char *argv[])
{
	struct input_event event;
	int fd;
	
	fd = open(argv[1], O_RDONLY);
	if(fd < 0){
		perror("open /dev/eventx");
		exit(1);
	}

	while(1){
		read(fd, &event, sizeof(struct input_event));	
		switch(event.type){
			case EV_SYN:
				printf("sync\n");
				break;
			case EV_KEY:
				if(event.value)
					printf("key:%d is donw\n", event.code);
				else
					printf("key:%d is up\n", event.code);
				break;
			case EV_REL:
				if(event.code == REL_X)
					printf("REL_X:%d\n", event.value);
				else if(event.code == REL_Y)
					printf("REL_Y:%d\n", event.value);
				break;
			case EV_ABS:
				if(event.code == ABS_X)
					printf("ABS_X:%d\n", event.value);
				else if(event.code == ABS_Y)
					printf("ABS_Y:%d\n", event.value);
				break;
			default:
				printf("Misc\n");
				break;
		}		
	}

	close(fd);

	return 0;
}





