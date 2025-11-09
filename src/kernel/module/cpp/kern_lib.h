/*
* Kernel lib - support basic C++ runtime functions
*/

#ifndef KERN_LIB_H
#define KERN_LIB_H

#ifdef __cplusplus
// #include <cstdarg>

extern "C" {
#else
// #include <stdarg.h>
#endif

void kmemset(void *dst, int c, unsigned int len);

void *kcmemcpy(void *dst, void *src, unsigned int len);

void *kcmemmove(void *dst, void *src, unsigned int len);

int kcmemcmp(void *p1, void *p2, unsigned int len);

void *kcmalloc(unsigned int size);

void *kcrealloc(void *mem, unsigned int size);

void kcfree(void *mem);

#ifdef __cplusplus
}
#endif

#endif
