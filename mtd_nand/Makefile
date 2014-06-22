LINUX :=/home/zhiyong/6410/smdk6410_lzy/src/linux3.4.24_ok
prefix ?=/nfsroot

obj-m += nand_dev.o
obj-m += nand_drv.o

all:
	make -C $(LINUX) M=`pwd` modules
clean:
	make -C $(LINUX) M=`pwd` modules clean
install:
	make -C $(LINUX) M=`pwd` modules_install INSTALL_MOD_PATH=$(prefix)


