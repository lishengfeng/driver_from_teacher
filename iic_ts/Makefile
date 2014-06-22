LINUX := /home/zhiyong/4412/exynos4412_lzy/exynos4412_lzy2/src/android/linux-3.5

prefix ?= /home/hero/nfsroot/rootfs

obj-m += ft5x06_drv.o

all:
	make -C $(LINUX) M=`pwd` modules
clean:
	make -C $(LINUX) M=`pwd` modules clean
install:
	make -C $(LINUX) M=`pwd` modules_install INSTALL_MOD_PATH=$(prefix)

