#include <linux/init.h>
#include <linux/module.h>

void my_printk(int no)
{
	printk("%s %s %d\n", __FUNCTION__, __FILE__, no);
}
EXPORT_SYMBOL(my_printk);

MODULE_LICENSE("GPL");
