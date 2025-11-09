#include <linux/init.h>
#include <linux/module.h>

#include <linux/printk.h>

MODULE_LICENSE("GPL");

MODULE_AUTHOR("LKMPG");
MODULE_DESCRIPTION("A sample driver");

static int __init init_hello_4(void)
{
    printk(KERN_INFO "Hello World 4\n");
    return 0;
}

static void __exit cleanup_hello_4(void)
{
    printk(KERN_INFO "Goodbye World 4\n");
}

module_init(init_hello_4);
module_exit(cleanup_hello_4);
