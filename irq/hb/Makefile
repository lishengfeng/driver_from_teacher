LINUX_PATH := /home/zhiyong/6410/smdk6410_lzy/src/linux3.4.24_ok
PREFIX := /nfsroot_week
#/nfsroot_week/lib/module/`uname -r`/extra

obj-m += workqueue_test.o

all:
	make -C $(LINUX_PATH) M=`pwd` modules
clean:
	make -C $(LINUX_PATH) M=`pwd` modules clean
install:
	make -C $(LINUX_PATH) M=`pwd` modules_install INSTALL_MOD_PATH=$(PREFIX)
