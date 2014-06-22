#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h> 

#include "s3c_block.h"

int ramdisk_trans(/*struct bio *b*/struct request_queue *q, char *buf, int offset, unsigned int bytes, unsigned int write)
{
//	struct ramdisk_st *rd = b->bi_bdev->bd_disk->private_data;
	struct ramdisk_st *rd = q->queuedata;

	if(offset + bytes > rd->size){
		printk("ramdisk is full\n");
		return -1;
	}

	if(write)
		memcpy(rd->content + offset, buf, bytes);
	else
		memcpy(buf, rd->content + offset, bytes);
	
	return bytes;
}

void ramdisk_bio_func(struct request_queue *q, struct bio *bio)
{
	struct bio_vec *vec;
	char *buf;
	int index;
	int ret = 0;

	//bio_for_each_segment(vec, bio, index){
    for(vec = bio_iovec_idx((bio), 0), i = 0; i < (bio)->bi_vcnt; vec++, i++){
		buf = kmap_atomic(vec->bv_page) + vec->bv_offset;	
		ret += ramdisk_trans(/*bio*/q, buf, bio->bi_sector * KERNEL_SEC_SIZE + ret, vec->bv_len, bio_data_dir(bio));	
		if(ret < 0)
			bio_endio(bio, -1);
		__kunmap_atomic(vec->bv_page);
	}
	bio_endio(bio, 0);
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
	.open = ramdisk_open,
	.release = ramdisk_release,
};

int setup_ramdisk(struct ramdisk_st *rd)
{
	int ret;

	rd->size = HARD_SEC_SIZE * HARD_SEC_NUM;

	rd->content = vzalloc(rd->size);
	if(!rd->content){
		ret = -ENOMEM;
		goto vzalloc_rd_error;
	}
#if 0
	spin_lock_init(&rd->lock);
	
	rd->ramdisk_queue = blk_init_queue(ramdisk_request_func, &rd->lock);
	if(!rd->ramdisk_queue){
		ret = -ENOMEM;
		goto blk_init_queue_error;
	}
#endif	
	rd->ramdisk_queue = blk_alloc_queue(GFP_KERNEL);
	if(!rd->ramdisk_queue){
		ret = -ENOMEM;
		goto blk_alloc_queue_error;
	}

	blk_queue_make_request(rd->ramdisk_queue, ramdisk_bio_func);	

	rd->ramdisk_queue->queuedata = rd;
	
	rd->disk = alloc_disk(MINOR_NUM);	
	if(!rd->disk){
		ret = -ENOMEM;
		goto alloc_disk_error;
	}

	rd->disk->major = rd->major;
	rd->disk->first_minor = FIRST_MINOR;
	sprintf(rd->disk->disk_name, "%s", "myramdisk");
	rd->disk->fops = &bops;
	rd->disk->queue = rd->ramdisk_queue;
	rd->disk->private_data = rd;
		
	set_capacity(rd->disk, HARD_SEC_SIZE * HARD_SEC_NUM / KERNEL_SEC_SIZE);

	add_disk(rd->disk);
	
	return 0;
alloc_disk_error:
	blk_cleanup_queue(rd->ramdisk_queue);
blk_alloc_queue_error:
	vfree(rd->content);
vzalloc_rd_error:
	return ret;
}

void clearup_ramdisk(struct ramdisk_st *rd)
{
	del_gendisk(rd->disk);
	put_disk(rd->disk);
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
