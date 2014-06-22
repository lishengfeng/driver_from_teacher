#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

int main(int argc, char *argv[])
{
	int so;
	int ret;
	struct sockaddr_in heraddr;

	so = socket(AF_INET, SOCK_DGRAM, 0);
	if(so < 0){
		perror("socket so");
		exit(1);
	}

	heraddr.sin_family = AF_INET;
	heraddr.sin_port = htons(1234);
	heraddr.sin_addr.s_addr = inet_addr(argv[1]);

	while(1){
		ret = sendto(so, "helloworld", 11, 0, (struct sockaddr *)&heraddr, sizeof(struct sockaddr_in));
		if(ret < 0){
			perror("sendto");
			exit(1);
		}
		sleep(1);
	}

	close(so);

	return 0;	
}







