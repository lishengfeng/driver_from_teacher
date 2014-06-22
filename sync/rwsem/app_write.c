#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// ./app_write 1234567 7
int main(int argc, char *argv[])
{
	int fd;
	int ret;

	fd = open("mdev", O_WRONLY);
	if(fd < 0){
		perror("open mdev");
		exit(1);
	}

	ret = write(fd, argv[1], atoi(argv[2]));	
	if(ret < 0){
		perror("write mdev");
		exit(1);
	}

	close(fd);

	return 0;
}





