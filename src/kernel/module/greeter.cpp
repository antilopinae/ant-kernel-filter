extern "C" {
#include <linux/kernel.h>
#include <linux/string.h>
}

#include <ant-kernel/greeter.h>

extern "C" {
void ant_cpp_greet(const char *name) {
    printk(KERN_INFO "CPP_HELPER: Hello, %s from C++ helper!\n", name);
}
}
