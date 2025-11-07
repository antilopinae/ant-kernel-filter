/*
* Kernel lib - support basic C++ runtime functions
*/

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "kern_lib.h"

void kcmemset(void *dst, int c, unsigned int len)
{
    memset(dst, c, len);
}

void *kcmemcpy(void *dst, void *src, unsigned int len)
{
    memcpy(dst, src, len);
    return dst;
}

void *kcmemmove(void *dst, void *src, unsigned int len)
{
    memmove(dst, src, len);
    return dst;
}

int kcmemcmp(void *p1, void *p2, unsigned int len)
{
    return memcmp(p1, p2, len);
}

void *kcmalloc(unsigned int size)
{
    return kmalloc(size, GFP_ATOMIC);
}

void *kcrealloc(void *mem, unsigned int size)
{
    return krealloc(mem, size, GFP_ATOMIC);
}

void kcfree(void *mem)
{
    kfree(mem);
}