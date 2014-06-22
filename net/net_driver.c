#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/module.h>

#if 0
socket
ifconfig
iwconfig
-----------------
kernel [net subsystem]
driver
-----------------
dm9000  
#endif

struct net_device *mynet;

//service network restart
//ifconfig week0 up
int nopen(struct net_device *ndev)
{
	printk("netdevice %s open\n", ndev->name);

	return 0;
}
//ifconfig week0 down
int nstop(struct net_device *ndev)
{
	printk("netdevice %s close\n", ndev->name);
	
	return 0;
}
//sendto write
//sendto()--->helloworld--[eth 14|ip 20|udp 8|helloworld]
netdev_tx_t nxmit(struct sk_buff *skb, struct net_device *ndev)
{
	printk("send ok\n");
	printk("%s\n", skb->data + 42);

	return NETDEV_TX_OK;
}

struct net_device_ops nops = {
	.ndo_open = nopen,
	.ndo_stop = nstop,
	.ndo_start_xmit = nxmit,
};

void setup_net(struct net_device *ndev)
{
	//把该设备初始化位以太网设备
	ether_setup(ndev);

	ndev->netdev_ops = &nops;	
	
	ndev->dev_addr[0] = 0x11;
	ndev->dev_addr[1] = 0x11;
	ndev->dev_addr[2] = 0x11;
	ndev->dev_addr[3] = 0x11;
	ndev->dev_addr[4] = 0x11;
	ndev->dev_addr[5] = 0x11;

	ndev->irq = IRQ_EINT(7);
}

static __init int net_test_init(void)
{
	int ret;

	//额外数据的大小，网卡名字，回调函数
	mynet = alloc_netdev(0, "week%d", setup_net);	
	if(!mynet)
		return -ENOMEM;

	ret = register_netdev(mynet);	
	if(ret)
		goto register_netdev_error;

	return 0;
register_netdev_error:
	free_netdev(mynet);
	return ret;
}

static __exit void net_test_exit(void)
{
	unregister_netdev(mynet);
	free_netdev(mynet);
}


module_init(net_test_init);
module_exit(net_test_exit);

MODULE_LICENSE("GPL");
