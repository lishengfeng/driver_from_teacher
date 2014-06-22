#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
	int led;
	int count = 100;

	led = open("led", O_RDWR);
	if(led < 0){
		perror("open led:");
		exit(1);
	}
	
	while(count--){
		write(led, "01", 3);
		sleep(1);
		write(led, "11", 3);
		sleep(1);
		write(led, "21", 3);
		sleep(1);
		write(led, "31", 3);
		sleep(1);
		write(led, "30", 3);
		sleep(1);
		write(led, "20", 3);
		sleep(1);
		write(led, "10", 3);
		sleep(1);
		write(led, "00", 3);
		sleep(1);
	}
	
	close(led);

	return 0;
}




