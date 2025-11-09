#include <linux/init.h>
#include <linux/module.h>

#include <linux/printk.h>

static int hello3data __initdata = 3;

static int __init hello3_init(void)
{
    printk(KERN_INFO "Hello world from kernel, %d\n", hello3data);

    return 0;
}

static void __exit hello3_exit(void)
{
    printk(KERN_INFO "Goodbye world, 3\n");
}

module_init(hello3_init);
module_exit(hello3_exit);

MODULE_LICENSE("GPL");