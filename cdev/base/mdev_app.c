#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(void)
{
	int fd;
	char *s = "helloworld";
	char buf[128];
	int ret;

	fd = open("mdev", O_RDWR);
	if(fd < 0){
		perror("");
		exit(1);
	}

	write(fd, s, strlen(s));	

	ret = read(fd, buf, 100);
	buf[ret] = 0;

	printf("content:%s\n", buf);
	

	close(fd);

	return 0;
}






