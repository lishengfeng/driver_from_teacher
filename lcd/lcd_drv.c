#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/fb.h>
#include "s3c_fb.h"
#include <linux/dma-mapping.h>

void s3c_lcd_init(struct s3cfb_info *s3cfb)
{
	u32 tmp;
		
	tmp = ioread32(s3cfb->m_v + MIFPCON);
	tmp &= MIFPCON_NOPASS;
	iowrite32(tmp, s3cfb->m_v + MIFPCON);

	tmp = ioread32(s3cfb->g_v + SPCON);
	tmp &= SPCON_RGB_MASK;
	tmp |= SPCON_RGB;
	iowrite32(tmp, s3cfb->g_v + SPCON);

	iowrite32(GPILCD, s3cfb->g_v + GPICON);
	iowrite32(GPJLCD, s3cfb->g_v + GPJCON);

	tmp = ioread32(s3cfb->g_v + GPECON);
	tmp &= GPE0OUT_MASK;
	tmp |= GPE0OUT;
	iowrite32(tmp, s3cfb->g_v + GPECON);	

	tmp = ioread32(s3cfb->g_v + GPEDAT);
	tmp |= GPE0HIGH;
	iowrite32(tmp, s3cfb->g_v + GPEDAT);

	tmp = ioread32(s3cfb->g_v + GPFCON);
	tmp &= GPF14OUT_MASK;
	tmp |= GPF14OUT;
	iowrite32(tmp, s3cfb->g_v + GPFCON);

	tmp = ioread32(s3cfb->g_v + GPFDAT);
	tmp |= GPF14HIGH;
	iowrite32(tmp, s3cfb->g_v + GPFDAT);

	tmp = VIDCON0_CLKVAL_F(14) | VIDCON0_CLKDIR;
	iowrite32(tmp, s3cfb->l_v + VIDCON0);

	tmp = VIDCON1_IHSYNC | VIDCON1_IVSYNC;
	iowrite32(tmp, s3cfb->l_v + VIDCON1);

	tmp = 0;
	iowrite32(tmp, s3cfb->l_v + VIDCON2);

	tmp = VIDTCON0_VBPD(1) | VIDTCON0_VFPD(1) | VIDTCON0_VSPW(9);
	iowrite32(tmp, s3cfb->l_v + VIDTCON0);

	tmp = VIDTCON1_HBPD(1) | VIDTCON1_HFPD(1) | VIDTCON1_HSPW(40);
	iowrite32(tmp, s3cfb->l_v + VIDTCON1);	

	tmp = VIDTCON2_LINEVAL(271) | VIDTCON2_HOZVAL(479);
	iowrite32(tmp, s3cfb->l_v + VIDTCON2);	

	tmp = WINCON0_HAWSWP | WINCON0_BPPMODE_F(5) | WINCON0_ENWIN_F;
	iowrite32(tmp, s3cfb->l_v + WINCON0);
	
	tmp = VIDOSD0A_LeftTopX(0) | VIDOSD0A_LeftTopY(0);
	iowrite32(tmp, s3cfb->l_v + VIDOSD0A);

	tmp = VIDOSD0B_RightBotX(479) | VIDOSD0B_RightBotY(271);
	iowrite32(tmp, s3cfb->l_v + VIDOSD0B);

	tmp = FB_WIDE * FB_HIGH >> 1;
	iowrite32(tmp, s3cfb->l_v + VIDOSD0C);

	iowrite32(s3cfb->fb_phy, s3cfb->l_v + VIDW00ADD0B0);
	iowrite32(s3cfb->fb_phy + VIDOSD0C * 4, s3cfb->l_v + VIDW00ADD1B0);
}

void s3c_lcd_on(struct s3cfb_info *s3cfb)
{
	u32 tmp;

	tmp = ioread32(s3cfb->l_v + VIDCON0);
	tmp |= VIDCON0_ENVID | VIDCON0_ENVID_F;
	iowrite32(tmp, s3cfb->l_v + VIDCON0);
}

void s3c_lcd_clear(struct s3cfb_info *s3cfb)
{
	int i, j;

	for(i = 0; i < s3cfb->fb.var.xres; i++)
		for(j = 0; j < s3cfb->fb.var.yres; j++)
			*((unsigned short *)(s3cfb->fb_virt) + j * s3cfb->fb.var.xres + i) = 0x1f;
}

void s3c_lcd_off(struct s3cfb_info *s3cfb)
{

}

void s3c_lcd_exit(struct s3cfb_info *s3cfb)
{

}

struct fb_ops s3cfb_ops = {
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
};

int palette[256] = {0};

void s3c_fb_info_init(struct s3cfb_info *s3cfb)
{
	s3cfb->fb.var.xres = FB_WIDE;                     
	s3cfb->fb.var.yres = FB_HIGH;
	s3cfb->fb.var.xres_virtual = FB_WIDE_V;             
	s3cfb->fb.var.yres_virtual = FB_HIGH_V;
	s3cfb->fb.var.xoffset = FB_WIDE_OFF;                  
	s3cfb->fb.var.yoffset = FB_HIGN_OFF;                  
	s3cfb->fb.var.bits_per_pixel = BIT_PER_FIX565;           
	s3cfb->fb.var.grayscale = 0;                
	s3cfb->fb.var.red.offset = RED_OFF;         
	s3cfb->fb.var.red.length = RED_BIT;         
	s3cfb->fb.var.red.msb_right = 0;
	s3cfb->fb.var.green.offset = GRE_OFF;       
	s3cfb->fb.var.green.length = GRE_BIT;       
	s3cfb->fb.var.green.msb_right = 0;       
	s3cfb->fb.var.blue.offset = BLU_OFF;
	s3cfb->fb.var.blue.length = BLU_BIT;
	s3cfb->fb.var.blue.msb_right = 0;

	s3cfb->fb.var.nonstd = 0;                   
	s3cfb->fb.var.activate = FB_ACTIVATE_NOW;                 
	s3cfb->fb.var.height = FB_HIGH;                   
	s3cfb->fb.var.width = FB_WIDE;                    
	s3cfb->fb.var.accel_flags = 0;              
	s3cfb->fb.var.pixclock = VCLK;                 
	s3cfb->fb.var.left_margin = LEFT_E;              
	s3cfb->fb.var.right_margin = RIGHT_E;             
	s3cfb->fb.var.upper_margin = UP_E;             
	s3cfb->fb.var.lower_margin = DOWN_E;
	s3cfb->fb.var.hsync_len = HSYNC_L;                
	s3cfb->fb.var.vsync_len = VSYNC_L;                
	s3cfb->fb.var.sync = FB_SYNC_COMP_HIGH_ACT;                     
	s3cfb->fb.var.vmode = FB_VMODE_NONINTERLACED;                    
	s3cfb->fb.var.rotate = 0;                   
	memcpy(s3cfb->fb.fix.id, "S3C-FB", 7);             
	s3cfb->fb.fix.smem_start = s3cfb->fb_phy;     
	s3cfb->fb.fix.smem_len = FB_WIDE * FB_HIGH * BYTE_PER_FIX565;    
	s3cfb->fb.fix.type = FB_TYPE_PACKED_PIXELS;           
	s3cfb->fb.fix.visual = FB_VISUAL_TRUECOLOR;         
	s3cfb->fb.fix.xpanstep = 0;       
	s3cfb->fb.fix.ypanstep = 0;       
	s3cfb->fb.fix.ywrapstep = 0;      
	s3cfb->fb.fix.line_length = FB_WIDE * BYTE_PER_FIX565;    

	s3cfb->fb.fbops = &s3cfb_ops;
	s3cfb->fb.screen_base = s3cfb->fb_virt;
	s3cfb->fb.screen_size = FB_WIDE * FB_HIGH * BYTE_PER_FIX565;
	s3cfb->fb.pseudo_palette = palette;	
}

int fb_probe(struct platform_device *pdev)
{
	int ret;
	struct s3cfb_info *fb0;
	struct resource *m_res, *g_res, *l_res;

	m_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	g_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	l_res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	if(!m_res || !g_res || !l_res)
		return -EBUSY;

	fb0 = kzalloc(sizeof(struct s3cfb_info), GFP_KERNEL);
	if(!fb0)
		return -ENOMEM;

	fb0->m_v = ioremap(m_res->start, m_res->end - m_res->start + 1);
	if(!fb0->m_v){
		ret = -ENOMEM;
		goto remap_mv_error;
	}
	
	fb0->g_v = ioremap(g_res->start, g_res->end - g_res->start + 1);
	if(!fb0->g_v){
		ret = -ENOMEM;
		goto remap_gv_error;
	}
	
	fb0->l_v = ioremap(l_res->start, l_res->end - l_res->start + 1);
	if(!fb0->l_v){
		ret = -ENOMEM;
		goto remap_lv_error;
	}

	//[R|G|B] 565 888 
	fb0->fb_virt = dma_alloc_coherent(NULL, FB_WIDE * FB_HIGH * BYTE_PER_FIX565, &fb0->fb_phy, GFP_KERNEL);
	if(!fb0->fb_virt){
		ret = -ENOMEM;
		goto alloc_fbmem_error;
	}	

	s3c_fb_info_init(fb0);	

	ret = register_framebuffer(&fb0->fb);
	if(ret)
		goto register_fb_error;		

	s3c_lcd_init(fb0);

	s3c_lcd_clear(fb0);

	s3c_lcd_on(fb0);

	platform_set_drvdata(pdev, fb0);
	
	return 0;
register_fb_error:
	dma_free_coherent(NULL, FB_WIDE * FB_HIGH * BYTE_PER_FIX565, fb0->fb_virt, fb0->fb_phy);	
alloc_fbmem_error:
	iounmap(fb0->l_v);
remap_lv_error:
	iounmap(fb0->g_v);
remap_gv_error:
	iounmap(fb0->m_v);
remap_mv_error:
	kfree(fb0);	
	return ret;
}

int fb_remove(struct platform_device *pdev)
{
	struct s3cfb_info *fb0 = platform_get_drvdata(pdev);

        s3c_lcd_exit(fb0);
	unregister_framebuffer(&fb0->fb);
	dma_free_coherent(NULL, FB_WIDE * FB_HIGH * BYTE_PER_FIX565, fb0->fb_virt, fb0->fb_phy);
        iounmap(fb0->l_v);
        iounmap(fb0->g_v);
        iounmap(fb0->m_v);
        kfree(fb0);

	return 0;
}

struct platform_driver drv = {
	.driver = {
		.name = "my-s3c-fb",
	},	
	.probe = fb_probe,
	.remove = fb_remove,
};

static __init int fb_test_init(void)
{
	return platform_driver_register(&drv);
}

static __exit void fb_test_exit(void)
{
	platform_driver_unregister(&drv);
}

module_init(fb_test_init);
module_exit(fb_test_exit);

MODULE_LICENSE("GPL");





