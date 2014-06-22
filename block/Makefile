LINUX := /home/zhiyong/6410/smdk6410_lzy/src/linux3.4.24_ok
#LINUX := /home/zhiyong/4412/exynos4412_lzy/exynos4412_lzy2/src/android/linux

obj-m += block_test1.o
obj-m += block_test2.o
obj-m += block_test3.o

all:
	make -C $(LINUX) M=`pwd` modules
clean:
	make -C $(LINUX) M=`pwd` modules clean
install:
	make -C $(LINUX) M=`pwd` modules_install INSTALL_MOD_PATH=/home/hero/nfsroot/rootfs
