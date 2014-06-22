#ifndef __S3C_BLOCK_H
#define __S3C_BLOCK_H

struct ramdisk_st {
        char *content;//表示硬盘的空间
        unsigned int size;//硬盘大小
        int user;//用户数量
        int major;//主设备号
        spinlock_t lock;//为请求队列分配锁
        struct request_queue *ramdisk_queue;//请求队列
        struct gendisk *disk;//表示一个磁盘
};

#define HARD_SEC_SIZE   512
#define HARD_SEC_NUM    102400
#define KERNEL_SEC_SIZE 512

#define MINOR_NUM       16
#define FIRST_MINOR     0

#endif
