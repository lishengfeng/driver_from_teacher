LINUX_PATH := /home/zhiyong/6410/smdk6410_lzy/src/linux3.4.24_ok

PREFIX ?= /nfsroot_week

obj-m += wdt_drv.o

all:
	make -C $(LINUX_PATH) M=`pwd` modules
	make -f app.mk
clean:
	make -C $(LINUX_PATH) M=`pwd` modules clean
	make -f app.mk clean
install:
	make -C $(LINUX_PATH) M=`pwd` modules_install INSTALL_MOD_PATH=$(PREFIX)
	
