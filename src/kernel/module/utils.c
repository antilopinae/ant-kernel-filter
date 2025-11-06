#include <linux/kernel.h>
#include <linux/string.h>
#include <ant-kernel/utils.h>

void ant_print_message(const char *prefix, const char *msg) {
    printk(KERN_INFO "%s: %s\n", prefix, msg);
}
