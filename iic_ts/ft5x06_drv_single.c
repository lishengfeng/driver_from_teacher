#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/slab.h>

struct ft5x06_ts_info {
	struct i2c_client *ic;
	struct workqueue_struct *workqueue;
	struct work_struct ft5x06_work;
	struct input_dev *idev;
	char buf[32];		
};

//#define DEBUG_TS

#if 0
在bsp中添加
static struct i2c_board_info smdk4x12_i2c_devs1[] __initdata = {
        {
                I2C_BOARD_INFO("ft5x0x_ts", (0x70 >> 1)),
        },
}
#endif

int ft5x06_read(struct i2c_client *ic, char reg, char *buf, int count)
{
	struct i2c_msg msg[2];

	msg[0].addr = ic->addr;
        msg[0].flags = 0;
        msg[0].len = 1;        
        msg[0].buf = &reg;     

	msg[1].addr = ic->addr;
        msg[1].flags = I2C_M_RD;
        msg[1].len = count;        
        msg[1].buf = buf;     

	/*这个函数会睡眠*/
	return i2c_transfer(ic->adapter, msg, 2);
}

void ft5x06_report(struct ft5x06_ts_info *ts)
{
	int tp = ts->buf[2] & 0xf;
	int x, y;

	x = ((ts->buf[3] & 0xf) << 8) | ts->buf[4];
	y = ((ts->buf[5] & 0xf) << 8) | ts->buf[6];

	/*注意：手指抬起时候检测的触点个数是0*/
	if(tp == 1){
		input_report_abs(ts->idev, ABS_X, x);
		input_report_abs(ts->idev, ABS_Y, y); 
		/*必须报告挤压事件*/ 
		input_report_abs(ts->idev, ABS_PRESSURE, 1);
		input_report_key(ts->idev, BTN_TOUCH, 1);
		input_sync(ts->idev);
	}else if(tp == 0){
		input_report_key(ts->idev, BTN_TOUCH, 0);
		input_sync(ts->idev);
	}	
}

void do_ft5x06_work(struct work_struct *work)
{
	int ret;
	struct ft5x06_ts_info *ts = container_of(work, struct ft5x06_ts_info, ft5x06_work);
	
	ret = ft5x06_read(ts->ic, 0, ts->buf, 7);
	if(ret < 0)
		return;
		
	ft5x06_report(ts);	
}

irqreturn_t do_touch(int no, void *data)
{
	struct ft5x06_ts_info *ts = data;

	queue_work(ts->workqueue, &ts->ft5x06_work);
	
	return IRQ_HANDLED;
}

int ts_probe(struct i2c_client *ic, const struct i2c_device_id *id)
{
	int ret;
	struct ft5x06_ts_info *ts;
	
	printk("Device %s probe addr:%x\n", ic->name, ic->addr);	

	ts = kzalloc(sizeof(struct ft5x06_ts_info), GFP_KERNEL);
	if(!ts)
		return -ENOMEM;

	ts->ic = ic;
	
	ts->workqueue = create_workqueue(ic->name);
	if(!ts->workqueue){
		ret = -EBUSY;
		goto alloc_queue_error;
	}

	INIT_WORK(&ts->ft5x06_work, do_ft5x06_work);	

	ts->idev = input_allocate_device();
	if(!ts->idev){
		ret = -ENOMEM;
		goto alloc_idev_error;
	}

	ts->idev->name = ic->name;
        ts->idev->phys = "/dev/input/eventx";
        ts->idev->uniq = "20140319";
	ts->idev->id.bustype = BUS_I2C;
        ts->idev->id.vendor = 0x110;
        ts->idev->id.product = 0x120;
        ts->idev->id.version = 0x119;

	set_bit(EV_SYN, ts->idev->evbit);
	set_bit(EV_KEY, ts->idev->evbit);
	set_bit(EV_ABS, ts->idev->evbit);

	set_bit(BTN_TOUCH, ts->idev->keybit);
	
	set_bit(ABS_X, ts->idev->absbit);
	set_bit(ABS_Y, ts->idev->absbit);
	set_bit(ABS_PRESSURE, ts->idev->absbit);
	
	input_set_abs_params(ts->idev, ABS_X, 0, 800, 0, 0);
	input_set_abs_params(ts->idev, ABS_Y, 0, 480, 0, 0);
	input_set_abs_params(ts->idev, ABS_PRESSURE, 0, 480, 0, 0);

	ret = input_register_device(ts->idev);
	if(ret < 0)
		goto register_idev_error;

	ret = request_irq(IRQ_EINT(14), do_touch, IRQ_TYPE_EDGE_FALLING, ic->name, ts);	
	if(ret < 0)
		goto request_irq_error;

	i2c_set_clientdata(ic, ts);

	return 0;
request_irq_error:
	input_unregister_device(ts->idev);
register_idev_error:
	input_free_device(ts->idev);	
alloc_idev_error:
	destroy_workqueue(ts->workqueue);
alloc_queue_error:
	kfree(ts);
	return ret;
}

int ts_remove(struct i2c_client *ic)
{
	struct ft5x06_ts_info *ts;
	
	ts = i2c_get_clientdata(ic);	
	
	free_irq(IRQ_EINT(14), ts);
	input_unregister_device(ts->idev);
	input_free_device(ts->idev);	
	flush_workqueue(ts->workqueue);
	destroy_workqueue(ts->workqueue);
	kfree(ts);

	return 0;
}

struct i2c_device_id ts_id_table[] = {
	//{"ft5x06_ts", 0},
	{"ft5x0x_ts", 0},
	{},
};

struct i2c_driver ts_drv = {
	.probe = ts_probe,
	.remove = ts_remove,
	.driver = {
		//.name = "ft5x06_ts"
		.name = "ft5x0x_ts"
	},
	.id_table = ts_id_table,
};

static __init int ts_test_init(void)
{
	return i2c_add_driver(&ts_drv);
}

static __exit void ts_test_exit(void)
{
	i2c_del_driver(&ts_drv);
}

module_init(ts_test_init);
module_exit(ts_test_exit);

MODULE_LICENSE("GPL");
