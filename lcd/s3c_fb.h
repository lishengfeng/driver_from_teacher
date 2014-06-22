#ifndef S3C_FB_H
#define S3C_FB_H

#include <linux/fb.h>
#include <asm/types.h>

struct s3cfb_info {
	void __iomem *m_v;//mifpcon
	void __iomem *g_v;//gpio
	void __iomem *l_v;//lcd
	char *fb_virt;//framebuffer v
	dma_addr_t fb_phy;//framebuffer p
	struct fb_info fb;
};
/************io phy****************/
//mifpcon
#define S3C_PA_FB_MIF	0x74108000
#define S3C_SZ_FB_MIF	SZ_4K
#define MIFPCON		0xC 
#define MIFPCON_NOPASS	~(1 << 3)
//gpio spcon
#define S3C_PA_FB_GPIO	0x7F008000 
#define S3C_SZ_FB_GPIO	SZ_4K

#define GPECON 		0x80
#define GPEDAT 		0x84
#define GPEPUD 		0x88
#define GPECONSLP 	0x8C
#define GPEPUDSLP 	0x90 

#define GPE0OUT		(1)
#define GPE0OUT_MASK	~(0xf)
#define GPE0HIGH	(1)
#define GPE0LOW		~(1)

#define GPFCON		0xA0
#define GPFDAT		0xA4
#define GPFPUD		0xA8
#define GPFECONSLP	0xAC
#define GPFPUDSLP	0xB0

#define GPF14OUT	(0x1 << 28)
#define GPF14OUT_MASK	~(0x3 << 28)
#define GPF14HIGH	(0x1 << 14)
#define GPF14LOW	~(0x1 << 14)

#define GPICON		0x100
#define GPIDAT		0x104
#define GPIPUD		0x108
#define GPICONSLP	0x10C
#define GPIPUDSLP	0x110

#define GPILCD		0xaaaaaaaa

#define GPJCON 		0x120
#define GPJDAT 		0x124
#define GPJPUD 		0x128
#define GPJCONSLP 	0x12C
#define GPJPUDSLP 	0x130 

#define GPJLCD		0xaaaaaa

#define SPCON		0x1A0
#define SPCON_RGB_MASK	~(3)
#define SPCON_RGB	(1)
//lcd
#define S3C_PA_FB	0x77100000
#define S3C_SZ_FB	SZ_4K
//common
#define VIDCON0 	0x00
#define VIDCON1 	0x04
#define VIDCON2 	0x08
#define VIDTCON0 	0x10
#define VIDTCON1 	0x14
#define VIDTCON2 	0x18
//window0
#define WINCON0 	0x20
#define VIDOSD0A 	0x40
#define VIDOSD0B 	0x44
#define VIDOSD0C 	0x48
#define VIDW00ADD0B0	0xA0
#define VIDW00ADD1B0	0xD0
#define VIDW00ADD2	0x100

//VIDCON0
#define VIDCON0_INTERLACE_F 	(1 << 29)
#define VIDCON0_VIDOUT(x)   	((x) << 26)
#define VIDCON0_L1_DATA16(x)	((x) << 23)
#define VIDCON0_L0_DATA16(x)	((x) << 20)
#define VIDCON0_PNRMODE(x)  	((x) << 17)
#define VIDCON0_CLKVALUP    	(1 << 16)
#define VIDCON0_CLKVAL_F(x) 	((x) << 6)
#define VIDCON0_VCLKFREE    	(1 << 5)
#define VIDCON0_CLKDIR	    	(1 << 4)
#define VIDCON0_CLKSEL_F(x) 	(1 << 2)
#define VIDCON0_ENVID	    	(1 << 1)
#define VIDCON0_ENVID_F	    	(1 << 0)
//VIDCON1
#define VIDCON1_LINECNT_MASK	(0x7ff << 16)
#define VIDCON1_FSTATUS_MASK	(1 << 15)
#define VIDCON1_VSTATUS_MASK 	(3 << 13)
#define VIDCON1_IVCLK 		(1 << 7)
#define VIDCON1_IHSYNC 		(1 << 6)
#define VIDCON1_IVSYNC 		(1 << 5)
#define VIDCON1_IVDEN 		(1 << 4)
//VIDTCON0
#define VIDTCON0_VBPDE(x) 	((x) << 24)
#define VIDTCON0_VBPD(x) 	((x) << 16)
#define VIDTCON0_VFPD(x) 	((x) << 8)
#define VIDTCON0_VSPW(x) 	((x) << 0)
//VIDTCON1
#define VIDTCON1_VFPDE(x) 	((x) << 24)
#define VIDTCON1_HBPD(x) 	((x) << 16)
#define VIDTCON1_HFPD(x) 	((x) << 8)
#define VIDTCON1_HSPW(x) 	((x) << 0)
//VIDTCON2
#define VIDTCON2_LINEVAL(x)	((x) << 11)
#define VIDTCON2_HOZVAL(x) 	((x) << 0)
//WINCON0
#define WINCON0_Narrow		(3 << 26)
#define WINCON0_ENLOCAL		(1 << 22)
#define WINCON0_BUFSTATUS_MASK 	(1 << 21)
#define WINCON0_BUFSEL 		(1 << 20)
#define WINCON0_BUFAUTOEN 	(1 << 19)
#define WINCON0_BITSWP 		(1 << 18)
#define WINCON0_BYTSWP 		(1 << 17)
#define WINCON0_HAWSWP 		(1 << 16)
#define WINCON0_InRGB		(1 << 13)
#define WINCON0_BURSTLEN(x) 	((x) << 9)
#define WINCON0_BPPMODE_F(x) 	((x) << 2)
#define WINCON0_ENWIN_F 	(1 << 0)
//VIDOSD0A
#define VIDOSD0A_LeftTopX(x)	((x) << 11) 
#define VIDOSD0A_LeftTopY(x) 	((x) << 0)
//VIDOSD0B
#define VIDOSD0B_RightBotX(x)	((x) << 11)
#define VIDOSD0B_RightBotY(x) 	((x) << 0)
//VIDOSD0C
#define VIDOSD0C_SIZE(x)		((x) << 0)
#define VIDOSD0C_OSDSIZE		((480 * 272) >> 1)
//VIDW00ADD0B0 
#define VIDW00ADD0B0_VBANK_F(x)		((x) << 24) 
#define VIDW00ADD0B0_VBASEU_F(x)	((x) << 0)
//VIDW00ADD1B0 
#define VIDW00ADD1B0_VBASEL_F(x)	((x) << 0)
//VIDW00ADD2 
#define VIDW00ADD2_OFFSIZE_F(x)		((x) << 13)
#define VIDW00ADD2_PAGEWIDTH_F(x)	((x) << 0)

/*************others***************/
#define FB_WIDE		480
#define FB_HIGH		272
#define FB_WIDE_OFF	0
#define FB_HIGN_OFF	0
#define FB_WIDE_V	(FB_WIDE - FB_WIDE_OFF)
#define FB_HIGH_V	(FB_HIGH - FB_HIGN_OFF)

#define BIT_PER_FIX565	16
#define BYTE_PER_FIX565	(BIT_PER_FIX565 >> 3)

#define RED_OFF		11
#define RED_BIT		5
#define GRE_OFF		5
#define GRE_BIT		6
#define BLU_OFF		0
#define BLU_BIT		5

#define VCLK		9000000
#define LEFT_E		2
#define RIGHT_E		2
#define UP_E		2
#define DOWN_E		2
#define VSYNC_L		10
#define HSYNC_L		41

#endif
