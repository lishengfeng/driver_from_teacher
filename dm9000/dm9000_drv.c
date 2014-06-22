#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include "dm9000.h"

void dm9000_write(struct net_device *ndev, unsigned char reg, unsigned char val)
{
	struct eth_priv *ep = netdev_priv(ndev);

	iowrite8(reg, ep->virt);//0x19000000
	iowrite8(val, ep->virt + 4);//0x19000004
}

unsigned char dm9000_read(struct net_device *ndev, unsigned char reg)
{
	struct eth_priv *ep = netdev_priv(ndev);

	iowrite8(reg, ep->virt);//0x19000000
	return ioread8(ep->virt + 4);//0x19000004
}

void s3c_dm9000_init(struct net_device *ndev)
{
	dm9000_write(ndev, 0x1f, 0x00);
	dm9000_write(ndev, 0x00, 0x01);
	dm9000_write(ndev, 0x00, 0x00);
	dm9000_write(ndev, 0xff, (1 << 7) | (1 << 0) | (1 << 1));

	dm9000_write(ndev, 0x10, ndev->dev_addr[0]);
	dm9000_write(ndev, 0x11, ndev->dev_addr[1]);
	dm9000_write(ndev, 0x12, ndev->dev_addr[2]);
	dm9000_write(ndev, 0x13, ndev->dev_addr[3]);
	dm9000_write(ndev, 0x14, ndev->dev_addr[4]);
	dm9000_write(ndev, 0x15, ndev->dev_addr[5]);	

	dm9000_write(ndev, 0x01, (1 << 2) | (1 << 3) | (1 <<5));
	dm9000_write(ndev, 0x05, 0x01);
}

void s3c_dm9000_exit(struct net_device *ndev)
{

}

int s3c_eth_send(struct sk_buff *sk, struct net_device *ndev)
{
	//sk->data
	//sk->len	
	int i;
	unsigned long flag;
	struct eth_priv *ep = netdev_priv(ndev);

	spin_lock_irqsave(&ep->lock, flag);

	if(dm9000_read(ndev, 0xfe) & 0x80){
		spin_unlock_irqrestore(&ep->lock, flag);
		return -1;
	}

	iowrite8(0xf8, ep->virt);
	for(i = 0; i < ((sk->len +1) >> 1); i++)
		iowrite16(((volatile unsigned short *)(sk->data))[i], ep->virt + 4);
	dm9000_write(ndev, 0xfd, (sk->len >> 8) & 0xff);
	dm9000_write(ndev, 0xfc, sk->len & 0xff);
	//开始发送
	dm9000_write(ndev, 0x02, 1);

	netif_stop_queue(ndev);

	spin_unlock_irqrestore(&ep->lock, flag);

	dev_kfree_skb(sk);	

	return 0;
}

int s3c_eth_recv(struct net_device *ndev)
{
	struct eth_priv *ep = netdev_priv(ndev);
	unsigned short status, length;
	struct sk_buff *sk;
	unsigned short *data;
	int i;

	while(1){
		dm9000_read(ndev, 0xf0);
		if((dm9000_read(ndev, 0xf0) & 0x03) != 0x01){
			return -1;
		}
		if(dm9000_read(ndev, 0xfe) & 0x80){
			return -1;
		}

		iowrite8(0xf2, ep->virt);
		status = ioread16(ep->virt + 4);
		length = ioread16(ep->virt + 4);

		sk = dev_alloc_skb(length);
		if(!sk)
			return -ENOMEM;

		data = (void *)skb_put(sk, length);

		for(i = 0; i < ((length + 1) >> 1); i++)
			data[i] = ioread16(ep->virt + 4);

		sk->protocol = eth_type_trans(sk, ndev);

		netif_rx(sk);	

		dm9000_write(ndev, 0xfe, 0x01);
	}
}

int if_recv_end(struct net_device *ndev)
{
	return dm9000_read(ndev, 0xfe) & 0x01;
}

int if_send_end(struct net_device *ndev)
{
	return dm9000_read(ndev, 0xfe) & 0x02;
}

void send_end(struct net_device *ndev)
{
        dm9000_write(ndev, 0xfe, 0x02);
	netif_wake_queue(ndev);
}
//-----------------------------------------------------
//不管网卡收到还是发送完成包，都会产生高点平中断
irqreturn_t do_net(int no, void *data)
{
	struct net_device *ndev = data;
	struct eth_priv *ep = netdev_priv(ndev);
	unsigned long flag;

	spin_lock_irqsave(&ep->lock, flag);

	if(if_send_end(ndev)){
		send_end(ndev);
	}else if(if_recv_end(ndev)){
		s3c_eth_recv(ndev);
	}else{
		spin_unlock_irqrestore(&ep->lock, flag);
		return IRQ_NONE;
	}

	spin_unlock_irqrestore(&ep->lock, flag);

	return IRQ_HANDLED;
}

int nopen(struct net_device *ndev)
{
	int ret;

        printk("netdevice %s open\n", ndev->name);

	ret = request_irq(ndev->irq, do_net, IRQF_TRIGGER_HIGH, ndev->name, ndev);
	if(ret)
		return -EBUSY;
	
	netif_start_queue(ndev);

	//对网卡进行初始化
	s3c_dm9000_init(ndev);

        return 0;
}
//ifconfig week0 down
int nstop(struct net_device *ndev)
{
        printk("netdevice %s close\n", ndev->name);

	s3c_dm9000_exit(ndev);
	netif_stop_queue(ndev);
	free_irq(ndev->irq, ndev);

        return 0;
}
//sendto write
//sendto()--->helloworld--[eth 14|ip 20|udp 8|helloworld]
netdev_tx_t nxmit(struct sk_buff *skb, struct net_device *ndev)
{
        //printk("send ok\n");
        //printk("%s\n", skb->data + 42);

	s3c_eth_send(skb, ndev);

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
}

int dm9000_probe(struct platform_device *pdev)
{
	int ret;
	u32 tmp;
	struct eth_priv *ep; 
	struct dm9000_platform_data *dm9000_data;
	struct resource *bw_res, *io_res, *irq_res;
	struct net_device *ndev;

	printk("Device %s is probe\n", pdev->name);

	bw_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	io_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if(!bw_res || !io_res || !irq_res)
		return -EBUSY;
	
	dm9000_data = pdev->dev.platform_data;

	//额外数据的大小，网卡名字，回调函数
        ndev = alloc_netdev(sizeof(struct eth_priv), "eth%d", setup_net);
        if(!ndev)
                return -ENOMEM;

	for(ret = 0; ret < 6; ret++){
        	ndev->dev_addr[ret] = dm9000_data->eth_addr[ret];
		printk("%x\n", ndev->dev_addr[ret]);
	}

	ndev->irq = irq_res->start;

        ret = register_netdev(ndev);
        if(ret)
                goto register_netdev_error;

	ep = netdev_priv(ndev);	

	ep->virt = ioremap(io_res->start, io_res->end - io_res->start + 1);
	if(!ep->virt){
		ret = -ENOMEM;
		goto ioremap_error;
	}

	ep->bw_v = ioremap(bw_res->start, bw_res->end - bw_res->start + 1);
	if(!ep->bw_v){
		ret = -ENOMEM;
		goto ioremap_bw_error;
	}	

	spin_lock_init(&ep->lock);

	//配置sromc为16位模式（因为网卡的是工作在16位模式）
	tmp = ioread32(ep->bw_v);
	tmp |= BW16;
	iowrite32(tmp, ep->bw_v);	

	platform_set_drvdata(pdev, ndev);

	return 0;
ioremap_bw_error:
	iounmap(ep->virt);
ioremap_error:
	unregister_netdev(ndev);
register_netdev_error:
	free_netdev(ndev);
	return ret;
}

int dm9000_remove(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct eth_priv *ep = netdev_priv(ndev);

	iounmap(ep->bw_v);
	iounmap(ep->virt);
	unregister_netdev(ndev);
	free_netdev(ndev);
	
	return 0;
}

struct platform_driver drv = {
	.driver = {
		.name = "s3c-dm9000",
	},
	.probe = dm9000_probe,
	.remove = dm9000_remove,
};

static __init int dm9000_test_init(void)
{
	return platform_driver_register(&drv);
}

static __exit void dm9000_test_exit(void)
{
	platform_driver_unregister(&drv);
}

module_init(dm9000_test_init);
module_exit(dm9000_test_exit);

MODULE_LICENSE("GPL");




