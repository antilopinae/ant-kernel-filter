#include <linux/module.h>
#include <linux/printk.h>

int init_module(void)
{
    pr_info("Hello world 1.\n");

    /* A nonzero return means init_module failed; module can't be loaded. */

    return 0;
}

void cleanup_module(void)
{
    pr_info("Goodbye world 1.\n");
}

MODULE_LICENSE("GPL");