/*基于请求队列，适合旋转(机械)硬盘*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h> 

#include "s3c_block.h"

void ramdisk_trans(struct request *req, char *buf, sector_t sec, unsigned int bytes, unsigned int write)
{
	struct ramdisk_st *rd = req->q->queuedata;
	unsigned int offset = sec * KERNEL_SEC_SIZE;

	if(offset + bytes > rd->size){
		printk("ramdisk is full\n");
		return;
	}

	if(write)
		memcpy(rd->content + offset, buf, bytes);
	else
		memcpy(buf, rd->content + offset, bytes);
}
//当请求对列中有请求时候，会掉用该函数，参数是请求队列
void ramdisk_request_func(struct request_queue *q)
{
	struct request *req;//每一个请求对应一个request
	int ret;

	while(1){
		//到请求对列获取一个请求
		req = blk_fetch_request(q);
		if(!req)
			break;	
		//scsi命令  睡眠请求  唤醒请求
		if(req->cmd_type != REQ_TYPE_FS){
			printk("Ignore a no-fs reqest\n");
			__blk_end_request_cur(req, -EPERM);
			continue;
		}
	again:
		//请求  该请求对应的buf   offset  buf中的数据长度  方向（读(0)\写(1)）
        //自定义函数
		ramdisk_trans(req, req->buffer, blk_rq_pos(req), blk_rq_cur_bytes(req), rq_data_dir(req));
		ret = __blk_end_request_cur(req, 0);
		if(ret)
			goto again;
	}	
}

int ramdisk_open(struct block_device *bd, fmode_t mode)
{
	struct ramdisk_st *rd = bd->bd_disk->private_data;

	rd->user++;

	return 0;
}

int ramdisk_release(struct gendisk *gd, fmode_t mode)
{
	struct ramdisk_st *rd = gd->private_data;
	
	rd->user--;

	return 0;
}

struct block_device_operations bops = {
	.open = ramdisk_open,//mount
	.release = ramdisk_release,//unmount
};

int setup_ramdisk(struct ramdisk_st *rd)
{
	int ret;

    //50M
	rd->size = HARD_SEC_SIZE * HARD_SEC_NUM;

    //kzalloc() kmalloc() 32 X pages

	rd->content = vzalloc(rd->size);
	if(!rd->content){
		ret = -ENOMEM;
		goto vzalloc_rd_error;
	}

	spin_lock_init(&rd->lock);
	
	//申请一个请求队列
	//第一个参数：该队列对应的请求函数，当对列有请求的时候内核会调用该函数
	rd->ramdisk_queue = blk_init_queue(ramdisk_request_func, &rd->lock);
	if(!rd->ramdisk_queue){
		ret = -ENOMEM;
		goto blk_init_queue_error;
	}
	//把rd保存到队列的私有成员中
	rd->ramdisk_queue->queuedata = rd;
	//每一个磁盘对应一个gendisk
	rd->disk = alloc_disk(MINOR_NUM);//16	
	if(!rd->disk){
		ret = -ENOMEM;
		goto alloc_disk_error;
	}

	rd->disk->major = rd->major;
	rd->disk->first_minor = FIRST_MINOR;//0
	// /dev/myramdisk
	sprintf(rd->disk->disk_name, "%s", "myramdisk");
	//块设备对应的操作
	rd->disk->fops = &bops;//open release
	//该磁盘的请求对列
	rd->disk->queue = rd->ramdisk_queue;
	//把rd保存磁盘的私有成员
	rd->disk->private_data = rd;
	//设置磁盘的大小，单位是内核扇区(512bytes)
	set_capacity(rd->disk, HARD_SEC_SIZE * HARD_SEC_NUM / KERNEL_SEC_SIZE);

	//blk_deivce---->gendisk
    //添加磁盘，注册块设备
	add_disk(rd->disk);
	
	return 0;
alloc_disk_error:
	blk_cleanup_queue(rd->ramdisk_queue);
blk_init_queue_error:
	vfree(rd->content);
vzalloc_rd_error:
	return ret;
}

void clearup_ramdisk(struct ramdisk_st *rd)
{
    //注销磁盘和块设备
	del_gendisk(rd->disk);
    //释放磁盘结构体
	put_disk(rd->disk);
    //注销请求队列
	blk_cleanup_queue(rd->ramdisk_queue);
	vfree(rd->content);
}

struct ramdisk_st *ramdisk;

static __init int block_test_init(void)
{
	int ret;

	ramdisk = kzalloc(sizeof(*ramdisk), GFP_KERNEL);	
	if(!ramdisk){
		ret = -ENOMEM;
		goto alloc_ramdisk_error;
	}

	ramdisk->major = register_blkdev(0, "my_ramdisk");
	if(ramdisk->major < 0){
		ret = ramdisk->major;
		goto register_blkdev_error;
	}

    //自定义函数，初始化并注册硬盘和块设备
	ret = setup_ramdisk(ramdisk);
	if(ret)
		goto setup_ramdisk_error;
	
	return 0;
setup_ramdisk_error:
	unregister_blkdev(ramdisk->major, "my_ramdisk");
register_blkdev_error:
	kfree(ramdisk);
alloc_ramdisk_error:
	return ret;
}

static __exit void block_test_exit(void)
{
	clearup_ramdisk(ramdisk);
	unregister_blkdev(ramdisk->major, "my_ramdisk");
	kfree(ramdisk);	
}

module_init(block_test_init);
module_exit(block_test_exit);

MODULE_LICENSE("GPL");
