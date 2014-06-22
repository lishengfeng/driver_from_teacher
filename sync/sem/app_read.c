#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
	int fd;
	int ret;
	char buf[1024];

	fd = open("mdev", O_RDONLY);
	if(fd < 0){
		perror("open mdev");
		exit(1);
	}

	ret = read(fd, buf, 1023);	
	if(ret < 0){
		perror("read mdev");
		exit(1);
	}

	buf[ret] = 0;
	
	printf("read:%s\n", buf);	

	close(fd);

	return 0;
}





