#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>

int main(void)
{
	unsigned short *v;
	int fd;
	int i, j;
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;

	fd = open("/dev/fb0", O_RDWR);
	if(fd < 0){
		perror("open /dev/fb0");
		exit(1);
	}

	ioctl(fd, FBIOGET_VSCREENINFO, &var);
	ioctl(fd, FBIOGET_FSCREENINFO, &fix);

	v = mmap(0, fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(!v){
		perror("mmap error");
		exit(1);
	}

	for(i = 0; i < var.xres; i++)
		for(j = 0; j < var.yres; j++)
			*(v + j * var.xres + i) = 0x1f << 11;
					//b11111

	for(i = var.xres / 5; i < var.xres / 5 * 4; i++)
		for(j = var.yres / 5; j < var.yres / 5 * 4; j++)
			*(v + j * var.xres + i) = 0x1f;

	munmap(v, fix.smem_len);
	close(fd);
	
	return 0;
}






